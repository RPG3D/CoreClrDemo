
rem DotNET/src/mono/

set SrcDir=%~dp0../../artifacts
set DstDir=D:/Code/MonoSDK_Temp

set BuildType=Debug

del /Q "%DstDir%"

xcopy "%SrcDir%/obj/mono/windows.x64.%BuildType%/out/include/mono-2.0" "%DstDir%/include" /E /I /Y
xcopy "%SrcDir%/obj/mono/windows.x64.%BuildType%/out/lib" "%DstDir%/lib" /E /I /Y
xcopy "%SrcDir%/obj/mono/windows.x64.%BuildType%/out/bin" "%DstDir%/bin" /E /I /Y
xcopy "%SrcDir%/bin/runtime/net10.0-windows-%BuildType%-x64" "%DstDir%/runtime" /E /I /Y
xcopy "%SrcDir%/bin/mono/windows.x64.%BuildType%/IL" "%DstDir%/runtime" /E /I /Y
