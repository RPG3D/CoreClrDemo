#include "ClrManager.h"

#include <iostream>

#include "Windows.h"

int ClrManager::LoadHostFxr()
{
    const char* HostFxrPath = R"(C:\Program Files\dotnet\host\fxr\9.0.9\hostfxr.dll)";

    HMODULE hModule = LoadLibraryA(HostFxrPath);

    if (hModule == NULL)
    {
        std::cout << "LoadLibraryA failed!\n";
        return -1;
    }

    InitForRuntimeConfig = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(hModule, "hostfxr_initialize_for_runtime_config");
    HostFxrGetDelegate = (hostfxr_get_runtime_delegate_fn)GetProcAddress(hModule, "hostfxr_get_runtime_delegate");
    CloseHostFxr = (hostfxr_close_fn)GetProcAddress(hModule, "hostfxr_close");

    return InitForRuntimeConfig != nullptr && HostFxrGetDelegate != nullptr && CloseHostFxr != nullptr;
}

load_assembly_and_get_function_pointer_fn ClrManager::LoadAssemblyAndGetFunctionPtr(const char_t* configPath)
{
    load_assembly_and_get_function_pointer_fn result = nullptr;

    hostfxr_handle ctx = nullptr;
    int rc = InitForRuntimeConfig(configPath, nullptr, &ctx);
    if (rc != 0 | ctx == nullptr)
    {
        CloseHostFxr(ctx);
        return nullptr;
    }

    rc = HostFxrGetDelegate(ctx, hdt_load_assembly_and_get_function_pointer, (void**)&result);

    if (rc != 0 || result == nullptr)
    {
        CloseHostFxr(ctx);
        return nullptr;
    }

    CloseHostFxr(ctx);

    return result;
}
