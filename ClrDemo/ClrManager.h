#pragma once

#include "dotnet/coreclr_delegates.h"
#include "dotnet/hostfxr.h"

class ClrManager
{
public:

    int LoadHostFxr();

    load_assembly_and_get_function_pointer_fn LoadAssemblyAndGetFunctionPtr(const char_t* configPath);
    
protected:
    
    hostfxr_initialize_for_runtime_config_fn InitForRuntimeConfig = nullptr;

    hostfxr_get_runtime_delegate_fn HostFxrGetDelegate = nullptr;

    hostfxr_close_fn CloseHostFxr = nullptr;
};
