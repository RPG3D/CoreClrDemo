// Android CoreCLR native host — minimal JNI library
//
// Flow: Java MainActivity loads libclrdemo.so → calls initRuntime() →
//       coreclr_initialize (low-level API, no hostfxr) → calls execEntryPoint() →
//       coreclr_create_delegate → calls ManagedClass.PrintMessage
//
// Key difference from desktop ClrAppDemo: Android has no hostfxr.
// We use the raw coreclr_initialize + coreclr_create_delegate API instead.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>

#include <jni.h>
#include <android/log.h>

#include "coreclrhost.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "CLRDEMO", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "CLRDEMO", __VA_ARGS__)

// ── Error writer: forwards CoreCLR diagnostics to logcat ────────

static void
coreclr_error_writer(const char* msg)
{
    __android_log_print(ANDROID_LOG_ERROR, "CORECLR", "%s", msg);
}

// ── Global state ────────────────────────────────────────────────

static char*  g_bundle_path = NULL;
static void*  g_coreclr_handle = NULL;
static unsigned int g_coreclr_domainId = 0;

// ── TPA: scan directory for all .dll, build colon-separated list ─

static char*
build_tpa(const char* dir)
{
    DIR* d = opendir(dir);
    if (!d) { LOGE("opendir failed: %s", dir); return NULL; }

    // Enough for ~200 DLLs × ~128 chars each
    char* tpa = (char*)calloc(1, 64 * 1024);
    if (!tpa) { closedir(d); return NULL; }

    struct dirent* e;
    while ((e = readdir(d)) != NULL)
    {
        const char* name = e->d_name;
        size_t len = strlen(name);
        if (len < 4 || strcasecmp(name + len - 4, ".dll") != 0)
            continue;

        if (tpa[0]) strcat(tpa, ":");
        strcat(tpa, dir);
        strcat(tpa, "/");
        strcat(tpa, name);
    }
    closedir(d);
    return tpa;
}

// ── JNI implementations ────────────────────────────────────────

JNIEXPORT jint JNICALL
Java_net_dot_clrdemo_MainActivity_initRuntime(
    JNIEnv* env, jclass clazz,
    jstring j_filesDir,
    jstring j_entryPointLibName)
{
    const char* filesDir = (*env)->GetStringUTFChars(env, j_filesDir, NULL);
    const char* entryPoint = (*env)->GetStringUTFChars(env, j_entryPointLibName, NULL);

    // Save bundle path for later use
    g_bundle_path = strdup(filesDir);

    // Build TRUSTED_PLATFORM_ASSEMBLIES
    char* tpa = build_tpa(filesDir);
    if (!tpa)
    {
        LOGE("Failed to build TPA");
        (*env)->ReleaseStringUTFChars(env, j_filesDir, filesDir);
        (*env)->ReleaseStringUTFChars(env, j_entryPointLibName, entryPoint);
        return -1;
    }
    LOGI("TPA built: %zu chars", strlen(tpa));

    // CoreCLR init properties (minimal set — TPA is enough for a demo)
    const char* keys[] = {
        "TRUSTED_PLATFORM_ASSEMBLIES",
        "APP_CONTEXT_BASE_DIRECTORY",
        "NATIVE_DLL_SEARCH_DIRECTORIES",
    };
    const char* values[] = {
        tpa,
        filesDir,
        filesDir,
    };
    int propCount = sizeof(keys) / sizeof(keys[0]);

    // Build app binary path (bundle_dir/entryPoint)
    char appPath[2048];
    snprintf(appPath, sizeof(appPath), "%s/%s", filesDir, entryPoint);

    // Register error writer for CoreCLR diagnostics (trace)
    coreclr_set_error_writer(coreclr_error_writer);

    LOGI("Calling coreclr_initialize(%s)", appPath);
    int rc = coreclr_initialize(
        appPath,           // exePath
        "AndroidClrDemo",  // appDomainFriendlyName
        propCount,
        keys,
        values,
        &g_coreclr_handle,
        &g_coreclr_domainId);

    free(tpa);
    LOGI("coreclr_initialize => 0x%x", rc);

    (*env)->ReleaseStringUTFChars(env, j_filesDir, filesDir);
    (*env)->ReleaseStringUTFChars(env, j_entryPointLibName, entryPoint);
    return rc;
}

JNIEXPORT jint JNICALL
Java_net_dot_clrdemo_MainActivity_execEntryPoint(
    JNIEnv* env, jclass clazz,
    jstring j_entryPointLibName)
{
    if (!g_coreclr_handle)
    {
        LOGE("CoreCLR not initialized");
        return -1;
    }

    const char* entryPoint = (*env)->GetStringUTFChars(env, j_entryPointLibName, NULL);

    // Use coreclr_create_delegate to get PrintMessage function pointer.
    // delegate signature: void PrintMessage(char*)
    typedef void (*PrintMessageFn)(char*);
    PrintMessageFn printFn = NULL;

    int rc = coreclr_create_delegate(
        g_coreclr_handle, g_coreclr_domainId,
        "ManagedDemo",                   // assembly name (no .dll)
        "ManagedDemo.ManagedClass",      // type full name (no assembly suffix)
        "PrintMessage",                  // method name
        (void**)&printFn);

    LOGI("coreclr_create_delegate => 0x%x, fn=%p", rc, printFn);

    if (rc == 0 && printFn)
    {
        printFn("Hello from Android native C!");
    }
    else
    {
        LOGE("Failed to create delegate for PrintMessage");
    }

    (*env)->ReleaseStringUTFChars(env, j_entryPointLibName, entryPoint);
    return rc;
}

JNIEXPORT void JNICALL
Java_net_dot_clrdemo_MainActivity_setEnv(
    JNIEnv* env, jclass clazz,
    jstring j_key, jstring j_value)
{
    const char* key = (*env)->GetStringUTFChars(env, j_key, NULL);
    const char* val = (*env)->GetStringUTFChars(env, j_value, NULL);
    setenv(key, val, 1);
    LOGI("setEnv: %s=%s", key, val);
    (*env)->ReleaseStringUTFChars(env, j_key, key);
    (*env)->ReleaseStringUTFChars(env, j_value, val);
}

JNIEXPORT void JNICALL
Java_net_dot_clrdemo_MainActivity_freeNativeResources(
    JNIEnv* env, jclass clazz)
{
    LOGI("freeNativeResources");

    if (g_coreclr_handle)
    {
        coreclr_shutdown(g_coreclr_handle, g_coreclr_domainId);
        g_coreclr_handle = NULL;
    }

    if (g_bundle_path)
    {
        free(g_bundle_path);
        g_bundle_path = NULL;
    }
}
