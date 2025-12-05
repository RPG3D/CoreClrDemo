

local DotNetDir = "./DotNETRuntime/win-x64"

if is_plat("macosx") then
    DotNetDir = "./DotNETRuntime/osx-arm64"
end 


add_rules("mode.debug", "mode.release")

target("ClrDemo")

    set_kind("binary")
    add_files("src/*.cpp")

    add_includedirs(DotNetDir)

    add_linkdirs(DotNetDir)

    add_links("nethost")
    add_rpathdirs(DotNetDir)
    