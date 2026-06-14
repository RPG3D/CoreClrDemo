
#include "nethost.h"
#include "coreclr_delegates.h"
#include "hostfxr.h"
#include <iostream>
#include <assert.h>

#if __APPLE__
#include <dlfcn.h>
#endif

#if _WIN32
#include <windows.h>
#endif

void (*PrintMessage)(char* InMsg);

int main(int argc, char** argv)
{
    // Self-contained flat layout: load hostfxr.dll directly from app dir
    const char_t* hostfxrPath = L"./hostfxr.dll";
    std::cout << "Loading hostfxr from: " << hostfxrPath << "\n";
    int rc = 0;

    void* libHostFxr = nullptr;
    hostfxr_initialize_for_runtime_config_fn init_config_fptr = nullptr;
    hostfxr_initialize_for_dotnet_command_line_fn init_cmd_fptr = nullptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr = nullptr;
    hostfxr_close_fn close_fptr = nullptr;

    // Error writer to capture full hostfxr error messages
    static auto error_writer = [](const char_t* msg) { std::wcerr << L"[hostfxr] " << msg << std::endl; };
    hostfxr_set_error_writer_fn set_error_writer_fptr = nullptr;

#if __APPLE__
    libHostFxr = dlopen(hostfxrPath, RTLD_LAZY);
    init_config_fptr = (hostfxr_initialize_for_runtime_config_fn)dlsym(libHostFxr, "hostfxr_initialize_for_runtime_config");
    init_cmd_fptr = (hostfxr_initialize_for_dotnet_command_line_fn)dlsym(libHostFxr, "hostfxr_initialize_for_dotnet_command_line");
    get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)dlsym(libHostFxr, "hostfxr_get_runtime_delegate");
    close_fptr = (hostfxr_close_fn)dlsym(libHostFxr, "hostfxr_close");
#endif

#if _WIN32
    libHostFxr = LoadLibraryW(hostfxrPath);
    HMODULE hModule = (HMODULE)libHostFxr;
    init_config_fptr = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(hModule, "hostfxr_initialize_for_runtime_config");
    init_cmd_fptr = (hostfxr_initialize_for_dotnet_command_line_fn)GetProcAddress(hModule, "hostfxr_initialize_for_dotnet_command_line");
    get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)GetProcAddress(hModule, "hostfxr_get_runtime_delegate");
    close_fptr = (hostfxr_close_fn)GetProcAddress(hModule, "hostfxr_close");
    set_error_writer_fptr = (hostfxr_set_error_writer_fn)GetProcAddress(hModule, "hostfxr_set_error_writer");
#endif

    // Capture full error messages from hostfxr
    if (set_error_writer_fptr)
        set_error_writer_fptr(error_writer);

    if(init_config_fptr && init_cmd_fptr && get_delegate_fptr && close_fptr)
    {
        std::cout<<"succeed init hostfxr\n";
    }
    else
    {
        std::cout<<"failed init hostfxr3\n";
    }

	// Self-contained via RuntimeConfig (UnrealSharp packaged mode pattern).
#if __APPLE__
    const char_t* assemblyPath = "./ManagedDemo.dll";
    const char_t* configPath = "./ManagedDemo.runtimeconfig.json";
    const char_t* dotnetRoot = ".";
#endif

#if _WIN32
    const char_t* assemblyPath = L"./ManagedDemo.dll";
    const char_t* configPath = L"./ManagedDemo.runtimeconfig.json";
#endif

    hostfxr_initialize_parameters initParams;
    initParams.size = sizeof(hostfxr_initialize_parameters);
    initParams.host_path = assemblyPath;
    initParams.dotnet_root = L"E:/Code/CoreClrDemo/ClrAppDemo/DotNetRuntime";

    hostfxr_handle handle = nullptr;
    const char_t* cmdArgs[] = { assemblyPath };
    rc = init_cmd_fptr(1, cmdArgs, &initParams, &handle);
    if (rc != 0 || handle == nullptr)
    {
        std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
        close_fptr(handle);
        return -1;
    }

    // Get the load assembly function pointer via runtime delegate
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
    rc = get_delegate_fptr(handle, hdt_load_assembly_and_get_function_pointer, (void**)&load_assembly_and_get_function_pointer);
    assert(rc == 0 && load_assembly_and_get_function_pointer != nullptr && "Get delegate failed");
    close_fptr(handle);

#if __APPLE__
    const char_t* dotnet_type = "ManagedDemo.ManagedClass, ManagedDemo";
    const char_t* methodName = "PrintMessage";
#endif

#if _WIN32
    const char_t* dotnet_type = L"ManagedDemo.ManagedClass, ManagedDemo";
    const char_t* methodName = L"PrintMessage";
#endif

    void* PrintMessageFuncPtr = nullptr;
    rc = load_assembly_and_get_function_pointer(assemblyPath, dotnet_type, methodName, UNMANAGEDCALLERSONLY_METHOD, nullptr, &PrintMessageFuncPtr);
    
    assert(PrintMessageFuncPtr != nullptr && "Failure: load_assembly_and_get_function_pointer()");

    PrintMessage = (void (*)(char *))PrintMessageFuncPtr;
    const char* msg = "Hello from native C++";
    PrintMessage((char*)msg);

    return 0;
}