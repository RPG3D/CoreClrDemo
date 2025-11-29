



add_rules("mode.debug")

local MonoSDK = "D:/Code/MonoSDK"

--if macos then set MonoSDK to user Documents dir
if is_plat("macosx") then
    MonoSDK = os.getenv("HOME") .. "/Documents/Code/MonoSDK"
end

target("MonoDemo")

    set_kind("binary")
    add_files("src/*.cpp")

    add_includedirs(MonoSDK .. "/include")
    add_linkdirs(MonoSDK .. "/lib")

    if is_plat("windows") then
        add_links("coreclr.import")
    elseif is_plat("macosx") then
        add_links("coreclr")
        add_rpathdirs("./")
    end

    