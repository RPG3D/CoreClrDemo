
rem place at DotNet source root directory

set SrcDir=%~dp0/artifacts
set DestDir=D:/Code/MonoSDK_Temp

set BuildType=Debug

del /Q "%DestDir%"

xcopy "%SrcDir%/obj/mono/windows.x64.%BuildType%/out/include/mono-2.0" "%DestDir%/include" /E /I /Y
xcopy "%SrcDir%/obj/mono/windows.x64.%BuildType%/out/lib" "%DestDir%/lib" /E /I /Y
xcopy "%SrcDir%/obj/mono/windows.x64.%BuildType%/out/bin" "%DestDir%/bin" /E /I /Y
xcopy "%SrcDir%/bin/runtime/net10.0-windows-%BuildType%-x64" "%DestDir%/runtime" /E /I /Y
xcopy "%SrcDir%/bin/mono/windows.x64.%BuildType%/IL" "%DestDir%/runtime" /E /I /Y
