

local DotNETDir = "./DotNETRuntime/win-x64"

if is_plat("macosx") then
    DotNETDir = "./DotNETRuntime/osx-arm64"
end 


add_rules("mode.debug", "mode.release")

target("ClrDemo")

    set_kind("binary")
    add_files("src/*.cpp")

    add_includedirs(DotNETDir)

    add_linkdirs(DotNETDir)

    add_links("nethost")
    add_rpathdirs("./")
    