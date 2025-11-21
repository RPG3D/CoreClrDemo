



add_rules("mode.debug")

local MonoSDK = "E:/Code/MonoSDK"

--if macos then set MonoSDK to user Documents dir
if is_plat("macosx") then
    MonoSDK = os.getenv("HOME") .. "/Documents/Code/MonoSDK"
end

target("MonoDemo")

    set_kind("binary")
    add_files("src/*.cpp")

    add_includedirs(MonoSDK .. "/include")
    add_linkdirs(MonoSDK .. "/lib")

    add_links(
        "coreclr.import",
        "mono-component-debugger-static",
        "mono-component-debugger-stub-static", 
        "mono-component-diagnostics_tracing-static",
        "mono-component-diagnostics_tracing-stub-static", 
        "mono-component-hot_reload-static",
        "mono-component-hot_reload-stub-static", 
        "mono-component-marshal-ilgen-static",
        "mono-component-marshal-ilgen-stub-static",
        "mono-profiler-aot",
        "monosgen-2.0"
    )
