## ADDED Requirements

### Requirement: CoreCLR 运行时初始化
JNI native 库 SHALL 通过 `coreclr_initialize` API 初始化 CoreCLR 运行时，传入必要的启动属性（TRUSTED_PLATFORM_ASSEMBLIES、APP_CONTEXT_BASE_DIRECTORY、NATIVE_DLL_SEARCH_DIRECTORIES），并返回初始化状态码。

#### Scenario: 成功初始化 CoreCLR
- **WHEN** Java 层调用 `initRuntime(filesDir, "ManagedDemo.dll", localDateTimeOffset)`
- **AND** `filesDir` 下存在所有必要的 BCL 程序集和 `libcoreclr.so`
- **THEN** `coreclr_initialize` 返回 0（S_OK）
- **AND** 生成有效的 `hostHandle` 和 `domainId`

#### Scenario: 程序集路径不存在
- **WHEN** `filesDir` 下缺少 `ManagedDemo.dll`
- **THEN** CoreCLR 初始化失败，返回非零 HRESULT

### Requirement: 托管方法调用
JNI native 库 SHALL 使用 `coreclr_create_delegate` 获取 `ManagedDemo.ManagedClass.PrintMessage` 的 native 函数指针，并通过该指针调用 managed 方法。

#### Scenario: 调用 PrintMessage 输出日志
- **WHEN** 运行时初始化成功后调用 `execEntryPoint("ManagedDemo.dll", args)`
- **THEN** 通过 `coreclr_create_delegate` 获取 `PrintMessage` 函数指针
- **AND** 调用该函数指针，传入字符串 "Hello from Android native"
- **AND** managed 方法执行，logcat 中出现 "Managed: PrintMessage in C#" 日志

### Requirement: 程序集外置加载
JNI native 库 SHALL 提供 `external_assembly_probe` 回调，在 CoreCLR 请求加载程序集时，从 bundle 目录（`filesDir`）通过 `mmap` 映射 .dll 文件。

#### Scenario: mmap 加载程序集
- **WHEN** CoreCLR 运行时请求加载 `System.Console.dll`
- **THEN** `external_assembly_probe` 回调被调用
- **AND** 从 bundle 路径找到该文件并 mmap 后返回 data 和 size
- **AND** logcat 输出 "Mapped System.Console.dll"

#### Scenario: 程序集不存在
- **WHEN** CoreCLR 运行时请求加载一个不存在的程序集
- **THEN** `external_assembly_probe` 返回 false
- **AND** CoreCLR 报告程序集加载失败

### Requirement: TPA 搜索路径构建
JNI native 库 SHALL 在初始化前扫描 bundle 目录下所有 .dll 文件，拼接为 TRUSTED_PLATFORM_ASSEMBLIES 分号分隔路径列表。

#### Scenario: 扫描目录构建 TPA
- **WHEN** bundle 目录下有 `ManagedDemo.dll`、`System.Private.CoreLib.dll` 等多个 .dll 文件
- **THEN** 构建的 TPA 字符串包含所有 .dll 文件的完整路径
- **AND** 各路径间以 `:` 分隔
