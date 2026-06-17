# AndroidClrDemo

Android 端 CoreCLR hosting 示例。与桌面端 `ClrAppDemo` 不同，Android 没有 hostfxr，必须使用底层 `coreclr_initialize` C API + JNI 桥接。

## Architecture: Android CoreCLR 初始化流程

```
┌──────────────────────────────────────────────────────────┐
│ Java (MainActivity)                                      │
│   System.loadLibrary("clrdemo")                          │
│   unzipAssets(zip) → filesDir/                           │
│   setEnv(HOME, TMPDIR)                                   │
│   initRuntime(filesDir, "ManagedDemo.dll")  ──JNI──┐     │
│   execEntryPoint("ManagedDemo.dll")  ──JNI──────────┤     │
└────────────────────────────────────────────────────┬┘     │
                                                     │      │
┌────────────────────────────────────────────────────┘      │
│ C (clrdemo.c)                                             │
│                                                           │
│   ① build_tpa(dir)                                        │
│     opendir → 遍历所有 .dll → 拼接 ":" 分隔的 TPA 字符串    │
│                                                           │
│   ② coreclr_set_error_writer(callback)                    │
│     注册错误回调 → CoreCLR 内部诊断输出到 logcat CORECLR tag  │
│                                                           │
│   ③ coreclr_initialize(                                   │
│       appPath,           // bundle_dir/ManagedDemo.dll    │
│       "AndroidClrDemo",  // app domain name               │
│       3,                 // property count                │
│       keys   = {"TRUSTED_PLATFORM_ASSEMBLIES",            │
│                 "APP_CONTEXT_BASE_DIRECTORY",              │
│                 "NATIVE_DLL_SEARCH_DIRECTORIES"},           │
│       values = {tpa, filesDir, filesDir})                 │
│     → hostHandle + domainId                               │
│                                                           │
│   ④ coreclr_create_delegate(                              │
│       hostHandle, domainId,                               │
│       "ManagedDemo",              // assembly name        │
│       "ManagedDemo.ManagedClass", // type (no assembly    │
│                                   //   suffix!)           │
│       "PrintMessage",                                    │
│       &fnPtr)                                            │
│     → 获得 native 函数指针                                 │
│                                                           │
│   ⑤ fnPtr("Hello from Android native C!")                 │
│     → ManagedClass.PrintMessage 执行                      │
│     → Console.WriteLine → logcat DOTNET tag               │
└───────────────────────────────────────────────────────────┘
```

### 与桌面端 hostfxr 的关键差异

| 桌面 (ClrAppDemo) | Android (AndroidClrDemo) |
|---|---|
| `hostfxr_initialize_for_dotnet_command_line` | `coreclr_initialize` |
| `load_assembly_and_get_function_pointer` delegate | `coreclr_create_delegate` |
| hostfxr 自动解析 TPA、runtimeconfig | 手动构建 TPA 字符串 |
| `get_hostfxr_path()` 找系统安装的 hostfxr | 无 hostfxr，直接 dlopen `libcoreclr.so` |
| 文件系统直接访问 .dll | .dll 在 assets.zip 中，运行时解压到 filesDir |
| `DOTNET_HOST_TRACE` 诊断 | `coreclr_set_error_writer` 回调 |
| `dotnet_root` 必须绝对路径 | JNI 传入 filesDir 绝对路径 |

### 自定义 DLL 加载

CoreCLR 通过 `TRUSTED_PLATFORM_ASSEMBLIES` (TPA) 属性知道所有可用的程序集。TPA 是一个冒号分隔的绝对路径列表：

```
/data/user/0/net.dot.clrdemo/files/ManagedDemo.dll:/data/user/0/.../System.Runtime.dll:...
```

构建方式：`opendir` 扫描 bundle 目录，过滤 `*.dll`，拼接路径。全部 190+ 个 BCL .dll 加上 `ManagedDemo.dll` 都在 assets.zip 中，首次启动时由 Java 侧解压到 `filesDir`。

> 注意：我们没有使用 `external_assembly_probe` (mmap 方案)，因为 TPA + 文件系统路径已足够。若需要从 APK 内直接 mmap 加载（不先解压），可参考官方 `monodroid-coreclr.c` 中的 `external_assembly_probe` 回调实现。

### Trace / 诊断机制

Android 底层 API 不支持 `DOTNET_HOST_TRACE`（那是 hostfxr 专属）。等效手段：

1. **`coreclr_set_error_writer`** — 注册回调，CoreCLR 内部错误/警告输出到 logcat `CORECLR` tag
2. **Native logcat** — `LOGI`/`LOGE` 宏输出到 `CLRDEMO` tag
3. **Managed `Console.WriteLine`** — 输出到 logcat `DOTNET` tag
4. **Tombstone** — native crash 时 Android 自动生成 `/data/tombstones/tombstone_XX`，包含完整寄存器 + backtrace

查看日志：
```bash
adb logcat -s CLRDEMO:I CORECLR:E DOTNET:I
```

---

## 打包流程

### 前置条件

- .NET SDK 11+（含 `android` workload：`dotnet workload install android`）
- Android SDK + NDK（`local.properties` 中配置 `sdk.dir`）
- JDK 21+（`JAVA_HOME`）

### 一键构建

```bash
cd AndroidClrDemo
build.bat
```

### 分步说明

**Step 1: Publish 托管代码**

```bash
cd ../ManagedDemo
dotnet publish -c Release --self-contained true \
    -p:RuntimeIdentifier=android-arm64 \
    -o ../AndroidClrDemo/publish/
```

产物：`publish/` 下 ~193 个文件：
- `ManagedDemo.dll` — 你的托管程序集
- `System.*.dll` — BCL 程序集
- `libcoreclr.so` — CoreCLR 运行时
- `libclrjit.so` — JIT 编译器
- `libSystem.*.so` — 系统 native 库

**Step 2: 生成 assets.zip**

```bash
# 所有 .dll 打包为 assets.zip（APK 运行时 Java 侧解压到 filesDir）
powershell Compress-Archive -Path 'publish/*.dll' -DestinationPath 'publish/assets.zip'
```

**Step 3: 放置预编译 .so**

```bash
# CoreCLR native .so 放到 jniLibs（Android 自动加载到 linker 搜索路径）
cp publish/libcoreclr.so app/src/main/jniLibs/arm64-v8a/
cp publish/libclrjit.so  app/src/main/jniLibs/arm64-v8a/
cp publish/libSystem.*.so app/src/main/jniLibs/arm64-v8a/
```

> `libclrdemo.so` 由 Gradle/CMake 自动编译，不需要手动复制。

**Step 4: Gradle 构建 APK**

```bash
./gradlew assembleDebug \
    -PdotnetRuntimeSrc="E:/Code/DotNet" \
    -PdotnetPublishDir="./publish"
```

产物：`app/build/outputs/apk/debug/app-debug.apk`

**Step 5: 部署测试**

```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n net.dot.clrdemo/.MainActivity
adb logcat -s CLRDEMO:I DOTNET:I CORECLR:E
```

预期输出：
```
I CLRDEMO : TPA built: 11170 chars
I CLRDEMO : coreclr_initialize => 0x0
I CLRDEMO : coreclr_create_delegate => 0x0, fn=0x...
I DOTNET  : Managed: static ManagedClass()
I DOTNET  : Managed: PrintMessage in C#, Hello from Android native C!
```

### APK 内部结构

```
HelloAndroid.apk
├── classes.dex              ← Java 字节码
├── lib/arm64-v8a/
│   ├── libclrdemo.so        ← JNI native 库（CMake 编译）
│   ├── libcoreclr.so        ← CoreCLR 运行时（publish）
│   ├── libclrjit.so         ← JIT 编译器（publish）
│   └── libSystem.*.so       ← 系统 native（publish）
├── assets/
│   └── assets.zip           ← 全部 managed .dll（运行时解压）
├── res/                     ← Android 资源
└── AndroidManifest.xml
```
