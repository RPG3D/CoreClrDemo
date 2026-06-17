## Context

CoreClrDemo 已有桌面端 CoreCLR host（`ClrAppDemo/`），通过 hostfxr 加载运行时并调用 managed 方法。Android 端不能使用 hostfxr——Android 上 CoreCLR 的入口是底层 C API：`coreclr_initialize` + `coreclr_create_delegate`（或 `coreclr_execute_assembly`），且必须通过 JNI 桥接 Java 与 native 代码。

官方 .NET Android 示例（`monodroid-coreclr.c` + `MonoRunner.java`）功能完整但较为复杂——包含 Instrumentation test harness、Mono 双轨支持等。本设计取其 CoreCLR 精华部分，做一个最小可运行的 demo。

**约束**：NDK 已安装（`D:/AndroidSDK/ndk/27.2.12479018`），.NET 10 已安装。

## Goals / Non-Goals

**Goals:**
- 实现最小 JNI native 库 `libclrdemo.so`，封装 CoreCLR 初始化和 `PrintMessage` 调用
- 实现最小 `MainActivity`，启动时解压 assets、加载 native 库、初始化 CoreCLR
- 复用现有 `ManagedDemo.dll`，输出 "Hello from Android native"
- 提供一键构建脚本（CMake + NDK + dotnet publish + APK 组装）

**Non-Goals:**
- 不实现 Instrumentation / test harness（官方 MonoRunner 的复杂部分）
- 不使用 Mono 运行时（只做 CoreCLR）
- 不支持 x86 / arm32（只做 arm64-v8a）
- 不集成 XHarness 测试框架
- 不做 AOT / R2R 编译

## Decisions

### 1. CoreCLR API：使用 `coreclr_initialize` + `coreclr_create_delegate`

**选择**：参考 `monodroid-coreclr.c` 使用 `coreclr_initialize` 初始化运行时，然后用 `coreclr_create_delegate` 获取 `ManagedClass.PrintMessage` 的函数指针并调用。

**备选方案**：`coreclr_execute_assembly`（运行托管 Main 入口）。更简单但不灵活（无法调用特定方法）。`coreclr_create_delegate` 与桌面端 `ClrAppDemo` 的 delegate 模式对齐，学习价值更高。

**实际实现**：先调用 `coreclr_initialize`（传入 TPA、APP_CONTEXT_BASE_DIRECTORY 等 property），运行时就绪后用 `coreclr_create_delegate` 获取 `PrintMessage` 的 native 函数指针，直接调用。

### 2. 构建系统：CMake + 自定义 shell 脚本

**选择**：CMakeLists.txt 编译 native .so，shell 脚本协调整体构建（dotnet publish + NDK 编译 + APK 组装）。xmake 对 Android NDK 的支持有限，不在此项目中使用。

```
AndroidClrDemo/
├── CMakeLists.txt          ← native 库构建
├── build.bat               ← 一键构建脚本
├── jni/
│   └── clrdemo.c           ← JNI native 入口
├── java/
│   └── net/dot/clrdemo/
│       └── MainActivity.java  ← Java 启动器
├── AndroidManifest.xml
└── publish/                ← dotnet publish 输出 (gitignored)
    ├── ManagedDemo.dll
    ├── System.Private.CoreLib.dll
    └── ... (BCL + CoreCLR .so)
```

### 3. Assets 加载方式：assets.zip 解压

**选择**：与官方示例一致——Runtime + BCL + ManagedDemo.dll 打包为 `assets.zip`，APK 运行时由 Java 侧解压到 `filesDir`。Native 侧配置 `external_assembly_probe` 回调从解压目录 mmap 程序集。

**备选方案**：使用 `host_runtime_contract.extract_bundle_probe`（直接从 APK 内 mmap）。更复杂，需要编译期生成 bundle 索引。demo 级别的 `assets.zip` 方案更直白。

### 4. TPA 构建：手动扫描目录

**选择**：Native 代码中扫描 bundle 目录下所有 `.dll` 构建 TRUSTED_PLATFORM_ASSEMBLIES 字符串。与官方示例一致，简单可靠。目录下约 190 个 dll 文件，用 calloc 分配 64KB 缓冲区构建路径列表。

### 5. Java 层：极简 Activity

**选择**：一个 `MainActivity` 继承 `Activity`（不继承 `Instrumentation`），在 `onCreate` 中加载 native 库 → 解压 assets → 调用 `initRuntime` → 延迟 1 秒后调用 managed 方法 → 显示返回结果。

**备选方案**：像官方那样用 `MonoRunner extends Instrumentation`。Instrumentation 模式是为 XHarness 测试框架设计的，demo 不需要。

## Risks / Trade-offs

- **硬编码路径**：与 ClrAppDemo 类似，构建脚本中硬编码 NDK、dotnet 路径。每个开发者需自行修改。→ 提供清晰的路径变量注释。
- **arm64-only**：未覆盖 x86_64 模拟器场景。→ 后续可扩展，当前优先 arm64 真机。
- **单一 managed 方法调用**：只调用 `PrintMessage`。后续可扩展 `Add` 和 delegate 回传等。
- **不依赖 AndroidAppBuilder**：官方用 MSBuild AndroidAppBuilder task 生成 C 模板（填充 `%AppContextPropertyCount%` 等占位符），我们手动构造 property 数组。更透明但需要硬编码 property 数量。
