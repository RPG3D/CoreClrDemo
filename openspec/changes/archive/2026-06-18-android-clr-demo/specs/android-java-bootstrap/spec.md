## ADDED Requirements

### Requirement: Native 库加载
`MainActivity` SHALL 在 `onCreate` 中通过 `System.loadLibrary` 加载 `libclrdemo.so` 原生库。

#### Scenario: 成功加载 native 库
- **WHEN** APK 安装后启动 Activity
- **AND** `lib/arm64-v8a/` 下存在 `libclrdemo.so`
- **THEN** `System.loadLibrary("clrdemo")` 成功执行
- **AND** native 库的 `JNI_OnLoad` 被调用

#### Scenario: native 库缺失
- **WHEN** APK 中缺少 `libclrdemo.so`
- **THEN** `System.loadLibrary` 抛出 `UnsatisfiedLinkError`
- **AND** Activity 显示错误信息后退出

### Requirement: Assets 解压
`MainActivity` SHALL 在运行时初始化前将 APK 内 `assets.zip` 解压到应用的 `filesDir`。

#### Scenario: 成功解压 assets
- **WHEN** Activity 调用 `unzipAssets(context, filesDir, "assets.zip")`
- **AND** `assets.zip` 包含 CoreCLR .so、BCL .dll、ManagedDemo.dll
- **THEN** 所有文件解压到 `filesDir` 的对应子目录中
- **AND** 文件结构与 zip 内一致（`lib/arm64-v8a/...`、`*.dll`）

### Requirement: CoreCLR 生命周期管理
`MainActivity` SHALL 依次执行：解压 assets → 设置环境变量 → 初始化运行时 → 延迟执行入口方法 → 返回结果。Activity 销毁时 SHALL 释放 native 资源。

#### Scenario: 完整的 CoreCLR 启动到执行
- **WHEN** Activity `onCreate` 被调用
- **THEN** 1) 解压 assets.zip 到 filesDir
- **AND** 2) 设置 HOME、TMPDIR 环境变量
- **AND** 3) 调用 `initRuntime(filesDir, entryPointLibName, localDateTimeOffset)`
- **AND** 4) 延迟 1 秒后调用 `execEntryPoint(entryPointLibName, args)`
- **AND** 5) UI 显示 "Mono Runtime returned: X" 文本

#### Scenario: Activity 销毁时释放资源
- **WHEN** Activity `onDestroy` 被调用
- **THEN** `freeNativeResources()` JNI 方法被调用
- **AND** native 侧释放 bundle_path、coreclr 句柄、mmap 的文件
