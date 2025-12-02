

local DotNETDir = "/usr/local/share/dotnet/packs/Microsoft.NETCore.App.Host.osx-arm64/10.0.0/runtimes/osx-arm64/native"

add_rules("mode.debug", "mode.release")

target("ClrDemo")

    set_kind("binary")
    add_files("src/*.cpp")

    add_includedirs(DotNETDir)
    add_includedirs("DotNetRuntime/inc")

    add_linkdirs(DotNETDir)

    add_links("nethost")
    add_rpathdirs("./")
    