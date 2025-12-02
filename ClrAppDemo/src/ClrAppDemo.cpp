
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <iostream>
#include <assert.h>

#ifdef  __APPLE__
#include <dlfcn.h>
#endif

int main(int argc, char** argv)
{
    char_t hostfxrPath[1024];
    size_t hostfxrPathSize = sizeof(hostfxrPath) / sizeof(char_t);
    int rc = get_hostfxr_path(hostfxrPath, &hostfxrPathSize, nullptr);
    if (rc != 0)
    {
        std::cout<<"failed init hostfxr\n";
        return -1;
    }

    void* libHostFxr = nullptr;
    hostfxr_initialize_for_runtime_config_fn init_fptr = nullptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr = nullptr;
    hostfxr_close_fn close_fptr = nullptr;

#ifdef  __APPLE__
    libHostFxr = dlopen(hostfxrPath, RTLD_LAZY);
    init_fptr = (hostfxr_initialize_for_runtime_config_fn)dlsym(libHostFxr, "hostfxr_initialize_for_runtime_config");
    get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)dlsym(libHostFxr, "hostfxr_get_runtime_delegate");
    close_fptr = (hostfxr_close_fn)dlsym(libHostFxr, "hostfxr_close");
#endif

    if(init_fptr && get_delegate_fptr && close_fptr)
    {
        std::cout<<"succeed init hostfxr\n";
    }
    else
    {
        std::cout<<"failed init hostfxr3\n";
    }

    const char* config_path = "./DotNetRuntime/dotnet.runtimeconfig.json";
    
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
    hostfxr_handle cxt = nullptr;
    rc = init_fptr(config_path, nullptr, &cxt);
    if (rc != 0 || cxt == nullptr)
    {
        std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
        close_fptr(cxt);
        return -1;
    }

    // Get the load assembly function pointer
    rc = get_delegate_fptr(cxt, hdt_load_assembly_and_get_function_pointer, (void**)&load_assembly_and_get_function_pointer);

    assert(rc == 0 && load_assembly_and_get_function_pointer != nullptr && "Get delegate failed");
    //close_fptr(cxt);

	const char* assemblyPath = "/Users/admin/Documents/Code/CoreClrDemo/ManagedDemo/bin/Debug/net10.0/ManagedDemo.dll";
    const char_t* dotnet_type = "ManagedDemo.ManagedClass, ManagedDemo";
    const char_t* methodName = "PrintMessage";

    void* Init = nullptr;
    rc = load_assembly_and_get_function_pointer(assemblyPath, dotnet_type, methodName, UNMANAGEDCALLERSONLY_METHOD, nullptr, &Init);
    
    assert(Init != nullptr && "Failure: load_assembly_and_get_function_pointer()");

    return 0;
}