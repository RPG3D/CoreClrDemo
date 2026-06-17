# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

CoreClrDemo is a learning project demonstrating how to host the .NET CoreCLR runtime from native C/C++ applications. It explores three hosting approaches: self-contained deployment via hostfxr (Windows/macOS), Mono runtime embedding, and Android CoreCLR via low-level C API (JNI + `coreclr_initialize`).

## Project Layout

```
ManagedDemo/              ← Shared C# assembly (net10.0) called by all native hosts
ClrAppDemo/
  src/ClrAppDemo.cpp      ← Native C++ host — self-contained CoreCLR via hostfxr
  src/{nethost,hostfxr,coreclr_delegates}.h  ← SDK headers from .NET runtime
  xmake.lua               ← xmake build config for the Windows native host
  DotNetRuntime/          ← Self-contained publish output (190 files flat, gitignored)
MonoAppDemo/
  src/MonoAppDemo.cpp     ← Alternative host using Mono runtime instead of CoreCLR
  xmake.lua               ← xmake build config (Windows + macOS)
AndroidClrDemo/
  app/src/cpp/clrdemo.c   ← JNI native library — CoreCLR init + delegate call
  app/src/cpp/CMakeLists.txt ← NDK build via CMake (integrated with Gradle)
  app/src/main/java/.../
    MainActivity.java      ← Minimal Activity bootstrap (unzip assets, init runtime)
  app/src/main/AndroidManifest.xml
  app/build.gradle.kts     ← Android Gradle config with CMake externalNativeBuild
  build.bat               ← One-shot: dotnet publish + Gradle assembleDebug
  publish/                ← Self-contained publish output for android-arm64 (gitignored)
  README.md               ← Detailed Android CLR init flow + packaging guide
CopyCoreCLR_Win64.bat     ← Script to copy CoreCLR artifacts from a dotnet/runtime build
CopyMonoSDK_Win64.bat     ← Script to copy Mono SDK artifacts from a dotnet/runtime build
Docs/SELFCONTAINED.md     ← Detailed self-contained deployment guide with debugging tips
```

## Build System

**Desktop hosts (Windows/macOS): xmake**

```bash
cd ClrAppDemo
xmake                    # Build CoreCLR host
xmake project -k vsxmake2026  # Generate VS solution

cd MonoAppDemo
xmake                    # Build Mono host
```

**Android host: Gradle + CMake**

```bash
cd AndroidClrDemo

# One-shot build (publish + assemble)
build.bat

# Or step by step:
cd ../ManagedDemo
dotnet publish -c Release --self-contained true \
    -p:RuntimeIdentifier=android-arm64 \
    -o ../AndroidClrDemo/publish/
cd ../AndroidClrDemo
./gradlew assembleDebug \
    -PdotnetRuntimeSrc="E:/Code/DotNet" \
    -PdotnetPublishDir="./publish"
```

Requires: Android SDK + NDK (path in `local.properties`), JDK 21+, .NET SDK with `android` workload.

**Managed DLL (all platforms):**

```bash
cd ManagedDemo
dotnet build

# Publish self-contained (Windows desktop)
dotnet publish -c Release --self-contained true -p:RuntimeIdentifier=win-x64 -o ../ClrAppDemo/DotNetRuntime/

# Publish self-contained (Android)
dotnet publish -c Release --self-contained true -p:RuntimeIdentifier=android-arm64 -o ../AndroidClrDemo/publish/
```

## Architecture: Three Hosting Approaches

### 1. Self-Contained CoreCLR via hostfxr (`ClrAppDemo`)

The C++ host loads `hostfxr.dll` directly from the app directory. Uses `hostfxr_initialize_for_dotnet_command_line` (not `_for_runtime_config`, which rejects self-contained apps). The `dotnet_root` parameter **must be an absolute path**. After initialization, it obtains a `load_assembly_and_get_function_pointer` delegate and calls `ManagedClass.PrintMessage`.

Key rules (see `Docs/SELFCONTAINED.md`):
- Self-contained needs NO `runtimeconfig.json`
- Flat layout — all 190 files in one directory
- `DOTNET_HOST_TRACE=1` for troubleshooting init failures

### 2. Mono Runtime Hosting (`MonoAppDemo`)

Uses `mono_jit_init()` / `mono_runtime_invoke()` instead of hostfxr. Demonstrates bidirectional managed↔native interop.

### 3. Android CoreCLR via low-level API (`AndroidClrDemo`)

Android has **no hostfxr**. The native library uses the raw `coreclr_initialize` + `coreclr_create_delegate` C API, called from Java via JNI.

**Init flow** (see `AndroidClrDemo/README.md` for full details):
1. Java extracts `assets.zip` (190+ managed DLLs) to `filesDir`
2. `build_tpa()` scans `filesDir/*.dll` → TRUSTED_PLATFORM_ASSEMBLIES string
3. `coreclr_set_error_writer()` — registers diagnostic callback
4. `coreclr_initialize()` with TPA + APP_CONTEXT_BASE_DIRECTORY + NATIVE_DLL_SEARCH_DIRECTORIES
5. `coreclr_create_delegate()` → function pointer to `ManagedClass.PrintMessage`
6. Call the delegate → managed `Console.WriteLine` output appears in logcat as `DOTNET` tag

**Key differences from desktop:**
- No `hostfxr` — uses `coreclr_initialize` directly
- JNI bridge — `MainActivity` loads `libclrdemo.so`
- TPA built manually by scanning the bundle directory
- `DOTNET_HOST_TRACE` **not available** — use `coreclr_set_error_writer` + logcat instead
- Native .so in APK `lib/arm64-v8a/`, managed .dll in `assets/assets.zip`
- Crash debug: Android tombstone at `/data/tombstones/`

## Key Files

- `ClrAppDemo/src/ClrAppDemo.cpp` — Desktop native host. Cross-platform with hardcoded absolute `dotnet_root`.
- `AndroidClrDemo/app/src/cpp/clrdemo.c` — Android JNI host. `build_tpa()`, `coreclr_initialize`, `coreclr_create_delegate`. Trace via `coreclr_set_error_writer`.
- `ManagedDemo/ManagedClass.cs` — Shared managed class. `[UnmanagedCallersOnly]` for direct C calling, `delegate* unmanaged` for native callback. Targets `net10.0`.

## Host Tracing & Debugging

### Desktop (hostfxr)

```cmd
set DOTNET_HOST_TRACE=1
set DOTNET_HOST_TRACEFILE=host_trace.log
set DOTNET_HOST_TRACE_VERBOSITY=4
```

Key error codes: `0x80008083` (hostpolicy not found), `0x80008087` (coreclr.dll not found), `0x80070057` (duplicate assemblies).

### Android (low-level API)

`DOTNET_HOST_TRACE` is hostfxr-only. Android equivalents:

| Mechanism | How |
|---|---|
| `coreclr_set_error_writer` | CoreCLR errors → logcat `CORECLR` tag |
| `LOGI`/`LOGE` macros | Native diagnostics → logcat `CLRDEMO` tag |
| `Console.WriteLine` | Managed output → logcat `DOTNET` tag |
| Tombstone | Native crash → `/data/tombstones/tombstone_XX` |

```bash
adb logcat -s CLRDEMO:I DOTNET:I CORECLR:E
```
