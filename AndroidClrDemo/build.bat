@echo off
setlocal enabledelayedexpansion
rem ============================================================
rem  AndroidClrDemo — Gradle-based build
rem  Standard Android NDK project structure
rem ============================================================
set PROJECT_DIR=%~dp0
set PUBLISH=%PROJECT_DIR%publish
set DOTNET_SRC=E:/Code/DotNet

echo === 1. Publish ManagedDemo (android-arm64, self-contained) ===
pushd %PROJECT_DIR%..\ManagedDemo
dotnet publish -c Release --self-contained true -p:RuntimeIdentifier=android-arm64 -o %PUBLISH% || (
    echo ERROR: dotnet publish failed.
    echo   Ensure android workload is installed: dotnet workload install android
    popd & exit /b 1
)
popd

echo === 2. Prepare Gradle assets ===

rem assets.zip: all managed .dll for CoreCLR to load at runtime
if exist "%PUBLISH%\assets.zip" del "%PUBLISH%\assets.zip"
powershell -Command ^
    "Compress-Archive -Path '%PUBLISH%\*.dll' -DestinationPath '%PUBLISH%\assets.zip' -Force"
copy "%PUBLISH%\assets.zip" "%PROJECT_DIR%app\src\main\assets\" >nul

rem jniLibs: prebuilt native .so (CoreCLR + system natives)
set JNILIBS=%PROJECT_DIR%app\src\main\jniLibs\arm64-v8a
copy "%PUBLISH%\libcoreclr.so"    "%JNILIBS%\" >nul
copy "%PUBLISH%\libclrjit.so"     "%JNILIBS%\" >nul
for %%f in (%PUBLISH%\libSystem.*.so) do copy "%%f" "%JNILIBS%\" >nul 2>nul
echo   jniLibs staged.

echo === 3. Gradle build (CMake + Java + APK) ===
set JAVA_HOME=C:\Program Files\Microsoft\jdk-21.0.11.10-hotspot
call "%PROJECT_DIR%gradlew.bat" assembleDebug ^
    -PdotnetRuntimeSrc="%DOTNET_SRC%" ^
    -PdotnetPublishDir="%PUBLISH%" ^
    || exit /b 1

echo.
echo === Done ===
set APK=%PROJECT_DIR%app\build\outputs\apk\debug\app-debug.apk
if exist "%APK%" (
    echo   APK: %APK%
    echo   Install: adb install -r "%APK%"
    echo   Run:     adb shell am start -n net.dot.clrdemo/.MainActivity
    echo   Logs:    adb logcat -s CLRDEMO:I DOTNET:I
) else (
    echo   APK not found — check Gradle output above.
)
endlocal
