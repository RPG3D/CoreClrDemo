# CoreCLR Self-Contained Deployment Guide

## Overview

Self-contained deployment bundles the CoreCLR runtime, JIT compiler, and all BCL assemblies alongside the native host application. This eliminates any dependency on a system-installed .NET runtime.

## Proven Recipe (Win x64, .NET 10.0.9)

### Step 1: Publish Managed DLL with Runtime

```bash
dotnet publish ManagedDemo/ManagedDemo.csproj \
    -c Release \
    --self-contained true \
    -p:RuntimeIdentifier=win-x64 \
    -o DotNetRuntime/
```

This produces **190 files** flat in `DotNetRuntime/`:
- `coreclr.dll` — CoreCLR runtime engine
- `clrjit.dll` — JIT compiler
- `hostfxr.dll` — Host FX resolver
- `hostpolicy.dll` — Host policy
- `System.Private.CoreLib.dll` — Foundational BCL
- `System.*.dll` — All BCL assemblies
- `ManagedDemo.dll`, `ManagedDemo.deps.json` — Your app

### Step 2: Write Native Host

Core files needed alongside your `.cpp`:
- `nethost.h` — from .NET SDK `packs/Microsoft.NETCore.App.Host.win-x64/<ver>/runtimes/win-x64/native/`
- `hostfxr.h` — same location
- `coreclr_delegates.h` — same location
- `nethost.lib` — same location (link dependency)

Key C++ code:

```cpp
#include "nethost.h"
#include "hostfxr.h"
#include "coreclr_delegates.h"

int main()
{
    // 1. Load hostfxr directly from app directory (NOT via get_hostfxr_path)
    HMODULE hHostfxr = LoadLibraryW(L"./hostfxr.dll");

    // 2. Get function pointers
    auto init_cmd = (hostfxr_initialize_for_dotnet_command_line_fn)
        GetProcAddress(hHostfxr, "hostfxr_initialize_for_dotnet_command_line");
    auto get_delegate = (hostfxr_get_runtime_delegate_fn)
        GetProcAddress(hHostfxr, "hostfxr_get_runtime_delegate");
    auto close_fn = (hostfxr_close_fn)
        GetProcAddress(hHostfxr, "hostfxr_close");
    auto set_error_writer = (hostfxr_set_error_writer_fn)
        GetProcAddress(hHostfxr, "hostfxr_set_error_writer");

    // 3. Set error writer for diagnostics
    set_error_writer([](const wchar_t* msg) { fwprintf(stderr, L"[hostfxr] %s\n", msg); });

    // 4. Initialize with dotnet exec mode + ABSOLUTE dotnet_root
    hostfxr_initialize_parameters params{};
    params.size = sizeof(params);
    params.host_path = L"./ManagedDemo.dll";
    params.dotnet_root = L"E:/Code/CoreClrDemo/ClrAppDemo/DotNetRuntime"; // MUST be absolute!

    const wchar_t* args[] = { L"./ManagedDemo.dll" };
    hostfxr_handle handle = nullptr;
    int rc = init_cmd(1, args, &params, &handle);

    // 5. Get assembly loader
    load_assembly_and_get_function_pointer_fn load_fn = nullptr;
    rc = get_delegate(handle, hdt_load_assembly_and_get_function_pointer,
                      (void**)&load_fn);
    close_fn(handle);

    // 6. Call managed method
    void* fn = nullptr;
    rc = load_fn(L"./ManagedDemo.dll",
                 L"ManagedDemo.ManagedClass, ManagedDemo",
                 L"PrintMessage",
                 UNMANAGEDCALLERSONLY_METHOD,
                 nullptr, &fn);
    auto PrintMessage = (void (*)(char*))fn;
    PrintMessage("Hello from native C++");
}
```

### Step 3: Layout

```
DotNetRuntime/
├── ClrDemo.exe                        ← native host
├── ManagedDemo.dll                    ← managed assembly
├── ManagedDemo.deps.json              ← dependency manifest
├── coreclr.dll                        ← CoreCLR runtime
├── clrjit.dll                         ← JIT compiler
├── hostfxr.dll                        ← host (loaded directly)
├── hostpolicy.dll                     ← host policy
├── System.Private.CoreLib.dll         ← BCL root
├── System.Runtime.dll                 ← BCL
├── System.Console.dll                 ← BCL
├── ... (all 190 files flat)
```

## Critical Rules (Discovered Through Debugging)

### 1. `dotnet_root` MUST be an absolute path

`hostpolicy_resolver::load()` checks `pal::is_path_fully_qualified(host_path)` and rejects relative paths with `CoreHostLibMissingFailure` (0x80008083). Source: `fxr/standalone/hostpolicy_resolver.cpp:38`.

❌ `L"."` — rejected
✅ `L"E:/Code/CoreClrDemo/ClrAppDemo/DotNetRuntime"` — accepted

### 2. Self-contained uses `hostfxr_initialize_for_dotnet_command_line`

NOT `hostfxr_initialize_for_runtime_config`. The latter rejects self-contained apps with "Initialization for self-contained components is not supported". Use the command-line initialization path with your DLL as the argument.

### 3. Load `hostfxr.dll` directly, NOT via `get_hostfxr_path()`

`get_hostfxr_path()` searches system install locations first and may return the system version. The system hostfxr may not work correctly with your local runtime layout. Load the app-local copy explicitly.

### 4. Self-contained needs NO `runtimeconfig.json`

`dotnet publish --self-contained` for a library project does NOT generate a `runtimeconfig.json`. The `hostfxr_initialize_for_dotnet_command_line` function constructs the runtime configuration internally from the DLL and deps.json.

### 5. Flat layout — no `shared/` or `host/` hierarchy needed

For self-contained deployments, hostfxr resolves everything from the flat directory (the directory containing the app). The `shared/Microsoft.NETCore.App/<ver>/` hierarchy is only needed for framework-dependent deployments.

### 6. `COREHOST_TRACE` / `DOTNET_HOST_TRACE` for debugging

Set these env vars to get detailed trace output:
```bash
set DOTNET_HOST_TRACE=1
set DOTNET_HOST_TRACEFILE=host_trace.log
set DOTNET_HOST_TRACE_VERBOSITY=4
```

Also register an error writer via `hostfxr_set_error_writer()` to capture runtime error messages.

## Platform Notes

### Android (planned)
```bash
dotnet publish -c Release --self-contained true \
    -p:RuntimeIdentifier=android-arm64 \
    -o DotNetRuntime/android-arm64/
```
Runtime files: `libcoreclr.so`, `libclrjit.so`, `libSystem.Native.so`, etc.

### iOS (planned, CoreCLR experimental in .NET 11)
iOS requires AOT (no JIT). Use ReadyToRun (R2R) via `crossgen2` or NativeAOT.

## Debugging & Diagnostics

### Host Tracing (CRITICAL for troubleshooting)

The .NET host components emit detailed trace logs when enabled. This was invaluable for solving:
- `0x80008083` — `hostpolicy.dll` not found or cannot be loaded (relative path rejected)
- `0x80008087` — CoreCLR path not resolved
- `0x80070057` — `E_INVALIDARG` from CoreCLR init (duplicate assemblies)

**Enable tracing (Windows):**
```cmd
set DOTNET_HOST_TRACE=1
set DOTNET_HOST_TRACEFILE=host_trace.log
set DOTNET_HOST_TRACE_VERBOSITY=4
```

**Also set in IDE** via project Debug properties → Environment. The trace file is written to the working directory.

**Key trace messages to look for:**
| Message | Meaning |
|---------|---------|
| `Using dotnet root parameter [<path>] as runtime location` | `dotnet_root` is being used |
| `Searching FX directory in [<path>]` | Where it's looking for the framework |
| `Chose FX version [<path>]` | Which framework directory was selected |
| `Executing as a self-contained app` | No `framework` section in runtimeconfig |
| `The expected hostpolicy.dll directory is [<path>]` | Where hostfxr looks for hostpolicy |
| `CoreCLR path = '<path>'` | Which coreclr.dll is being loaded |
| `Property NATIVE_DLL_SEARCH_DIRECTORIES` | Native DLL search paths |
| `Property TRUSTED_PLATFORM_ASSEMBLIES` | All assemblies loaded into the TPA |
| `Could not resolve CoreCLR path` | coreclr.dll not found at expected location |

### Error Writer

Register an error writer to capture hostfxr errors before initialization:
```cpp
auto set_error_writer = (hostfxr_set_error_writer_fn)
    GetProcAddress(hHostfxr, "hostfxr_set_error_writer");
set_error_writer([](const wchar_t* msg) {
    fwprintf(stderr, L"[hostfxr] %s\n", msg);
});
```

### Common Error Codes

| Code | Name | Root Cause |
|------|------|-----------|
| `0x80008083` | `CoreHostLibMissingFailure` | hostpolicy.dll not found OR relative `dotnet_root` path. Use **absolute** path. |
| `0x80008087` | `CoreHostLibMissingFailure` (variant) | coreclr.dll not found in expected location |
| `0x80070057` | `E_INVALIDARG` | Duplicate assemblies in TPA, version mismatch, or malformed config |

### Source-Level Debugging

The host source code at `dotnet/runtime/src/native/corehost/` is essential reading:

- `fxr/standalone/hostpolicy_resolver.cpp:38` — **Rejects relative paths** for hostpolicy.dll (`is_path_fully_qualified` check)
- `fxr/fx_muxer.cpp:140` — Error message "An error occurred while loading required library"
- `fxr/fx_muxer.cpp:762` — Same for non-execute path
- `error_codes.h` — All error code definitions

## References
- `hostpolicy_resolver.cpp` (standalone): `dotnet/runtime/src/native/corehost/fxr/standalone/hostpolicy_resolver.cpp`
- `fx_muxer.cpp`: `dotnet/runtime/src/native/corehost/fxr/fx_muxer.cpp`
- Host tracing docs: `dotnet/runtime/docs/design/features/host-tracing.md`
- Error codes: `dotnet/runtime/src/native/corehost/error_codes.h`
