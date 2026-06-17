## Why

CoreClrDemo 目前已演示了桌面端（Windows/macOS）CoreCLR hosting，但缺少 Android 平台示例。Android 端 CoreCLR hosting 与桌面端差异巨大——无法使用 hostfxr，必须通过 JNI + 底层 `coreclr_initialize` API。增加 Android 示例可以完善项目的学习价值，展示 CoreCLR 在移动端的真实启动流程。

## What Changes

- 新增 `AndroidClrDemo/` 项目：JNI native 库 (`libclrdemo.so`) + Java 引导代码
- 使用 CMake + NDK 构建 native 部分（xmake 不支持 Android NDK 构建）
- Native 代码直接调用 `coreclr_initialize` / `coreclr_execute_assembly`（不依赖 hostfxr）
- Java 侧提供极简 `MainActivity`，启动时解压 assets 并初始化 CoreCLR 运行时
- Managed 侧复用现有 `ManagedDemo.dll`，调用 `ManagedClass.PrintMessage`
- 发布流程：`dotnet publish` 产生 android-arm64 BCL + `ManagedDemo.dll`，打包为 assets 放入 APK

## Capabilities

### New Capabilities

- `android-native-host`: Android JNI native 库，封装 CoreCLR 初始化、assembly 加载、managed 方法调用
- `android-java-bootstrap`: Java/Kotlin Activity 引导层，负责解压 assets、加载 native 库、启动 CoreCLR

### Modified Capabilities

<!-- None — this is an entirely new platform demo, no existing specs change -->

## Impact

- 新增目录 `AndroidClrDemo/`（native C + CMakeLists.txt + Java 源码 + build 脚本）
- 复用 `ManagedDemo/`（C# 端无需修改，只需为 android-arm64 发布）
- 不影响现有 `ClrAppDemo/` 和 `MonoAppDemo/`
