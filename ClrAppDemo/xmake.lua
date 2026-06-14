

add_rules("mode.debug", "mode.release")

target("ClrDemo")

    set_kind("binary")
    add_files("src/*.cpp")
    set_rundir(DotNetDir)
    add_runenvs("DOTNET_HOST_TRACE", "1")
    add_runenvs("DOTNET_HOST_TRACEFILE", "E:/Code/CoreClrDemo/ClrAppDemo/DotNetRuntime/host_trace.log")
    add_runenvs("DOTNET_HOST_TRACE_VERBOSITY", "4")
    add_runenvs("COREHOST_TRACE", "1")
    add_runenvs("COREHOST_TRACEFILE", "E:/Code/CoreClrDemo/ClrAppDemo/DotNetRuntime/host_trace.log")
    add_runenvs("COREHOST_TRACE_VERBOSITY", "4")

    add_includedirs("src")

    add_linkdirs("src")

    add_links("nethost")
    