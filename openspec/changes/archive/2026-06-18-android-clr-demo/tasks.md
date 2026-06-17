## 1. 项目骨架搭建

- [x] 1.1 创建 `AndroidClrDemo/` 目录结构（jni/、java/net/dot/clrdemo/、publish/）
- [x] 1.2 编写 `CMakeLists.txt`：编译 `libclrdemo.so`，链接 `libcoreclr.so`、`log`、`z`
- [x] 1.3 编写 `AndroidManifest.xml`（包名 `net.dot.clrdemo`，MainActivity 入口）
- [x] 1.4 编写 `build.bat` 构建脚本骨架（路径变量：NDK、DOTNET_SRC、OUTPUT）

## 2. Native JNI 库 (libclrdemo.so)

- [x] 2.1 实现 `build_tpa()`：扫描 bundle 目录下所有 .dll，拼接 TPA 字符串
- [x] 2.2 实现 `external_assembly_probe()`：从 bundle 路径 mmap 程序集文件
- [x] 2.3 实现 `Java_net_dot_clrdemo_MainActivity_initRuntime()` JNI 方法（调用 `coreclr_initialize`）
- [x] 2.4 实现 `Java_net_dot_clrdemo_MainActivity_execEntryPoint()` JNI 方法（调用 `coreclr_create_delegate` → `PrintMessage`）
- [x] 2.5 实现 `Java_net_dot_clrdemo_MainActivity_freeNativeResources()` JNI 方法

## 3. Java 引导层 (MainActivity)

- [x] 3.1 编写 `MainActivity.java`：加载 `clrdemo` native 库，声明 native 方法
- [x] 3.2 实现 `unzipAssets()`：解压 `assets.zip` 到 `filesDir`
- [x] 3.3 实现 `onCreate` 完整启动流程：解压 → 初始化 → 延迟执行 → 显示结果
- [x] 3.4 实现 `onDestroy` 中调用 `freeNativeResources()`

## 4. Managed 端发布

- [x] 4.1 ManagedDemo 无需 TFM 修改 — `net10.0` 生成的 DLL 是 CPU-agnostic IL，可直接用于 Android
- [x] 4.2 获取 `libcoreclr.so` 和相关 native .so — build.bat 从 .NET runtime artifacts 复制
- [x] 4.3 获取 BCL .dll — build.bat 中 `dotnet publish --self-contained -r android-arm64`
- [x] 4.4 编写打包脚本 — build.bat 中 PowerShell Compress-Archive 打包 assets.zip

## 5. 集成验证

- [x] 5.1 手动执行完整构建流程验证无编译错误
- [x] 5.2 将 APK 部署到 Android arm64 设备/模拟器并运行
- [x] 5.3 验证 logcat 输出 "Managed: PrintMessage in C#, Hello from Android native"
- [x] 5.4 更新 `CLAUDE.md` 添加 AndroidClrDemo 构建说明
