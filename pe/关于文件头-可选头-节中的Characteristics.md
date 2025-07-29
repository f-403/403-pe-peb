## 以下是完整对比，包括：

* ✅ `File Header` 中的 `Characteristics`
* ✅ `Section Header` 中的 `Characteristics`
* ✅ `Optional Header` 中的 `DllCharacteristics`

我会用表格对比功能、作用范围、标志含义，并提供中英文对照，以及官方 MSDN 链接，便于你学习、查阅和后续写加载器或壳工具时参考。

---

## 三个 Characteristics 字段对比概览

| 字段名                  | 所属结构                    | 描述对象                   | 用途                      | 示例                    | MSDN 链接                                                                                            |
| -------------------- | ----------------------- | ---------------------- | ----------------------- | --------------------- | -------------------------------------------------------------------------------------------------- |
| `Characteristics`    | `IMAGE_FILE_HEADER`     | 整个 PE 文件               | 描述文件类型（EXE、DLL）、是否可重定位等 | `0x2000` 表示是 DLL      | [MSDN 说明](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_file_header)    |
| `Characteristics`    | `IMAGE_SECTION_HEADER`  | 单个节（如 `.text`、`.data`） | 描述节的读写执行权限和内容           | `0x60000020` 可读可执行代码段 | [MSDN 说明](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_section_header) |
| `DllCharacteristics` | `IMAGE_OPTIONAL_HEADER` | 安全机制与兼容性               | 控制 ASLR、DEP、CFG 等运行特性   | `0x0040` 启用 ASLR      | [MSDN 说明](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#dll-characteristics)     |

---

## 1. `IMAGE_FILE_HEADER.Characteristics`（PE 文件标志）

[→ 官方 MSDN 文档](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_file_header)

| 值        | 宏定义                              | 中文含义             |
| -------- | -------------------------------- | ---------------- |
| `0x0001` | `IMAGE_FILE_RELOCS_STRIPPED`     | 重定位信息被移除（不可重定基址） |
| `0x0002` | `IMAGE_FILE_EXECUTABLE_IMAGE`    | 可执行映像（EXE 或 DLL） |
| `0x0004` | `IMAGE_FILE_LINE_NUMS_STRIPPED`  | 行号信息被移除（调试信息）    |
| `0x0008` | `IMAGE_FILE_LOCAL_SYMS_STRIPPED` | 本地符号被移除（调试信息）    |
| `0x0020` | `IMAGE_FILE_LARGE_ADDRESS_AWARE` | 大地址支持（超过 2GB）    |
| `0x0100` | `IMAGE_FILE_32BIT_MACHINE`       | 适用于 32 位系统       |
| `0x0200` | `IMAGE_FILE_DEBUG_STRIPPED`      | 调试信息被移除          |
| `0x2000` | `IMAGE_FILE_DLL`                 | 是 DLL 文件         |
| `0x4000` | `IMAGE_FILE_SYSTEM`              | 系统文件（驱动等）        |
| `0x8000` | `IMAGE_FILE_UP_SYSTEM_ONLY`      | 只能在单处理器上运行       |

---

## 2. `IMAGE_SECTION_HEADER.Characteristics`（节的属性）

[→ 官方 MSDN 文档](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_section_header)

### 权限相关（组合使用）

| 值            | 宏定义                     | 中文含义 |
| ------------ | ----------------------- | ---- |
| `0x20000000` | `IMAGE_SCN_MEM_EXECUTE` | 可执行  |
| `0x40000000` | `IMAGE_SCN_MEM_READ`    | 可读   |
| `0x80000000` | `IMAGE_SCN_MEM_WRITE`   | 可写   |

### 节内容类型（也可组合）

| 值            | 宏定义                                | 中文含义           |
| ------------ | ---------------------------------- | -------------- |
| `0x00000020` | `IMAGE_SCN_CNT_CODE`               | 包含代码           |
| `0x00000040` | `IMAGE_SCN_CNT_INITIALIZED_DATA`   | 包含已初始化的数据      |
| `0x00000080` | `IMAGE_SCN_CNT_UNINITIALIZED_DATA` | 包含未初始化数据（.bss） |
| `0x01000000` | `IMAGE_SCN_ALIGN_4096BYTES`        | 节按 4096 字节对齐   |
| `0x00000008` | `IMAGE_SCN_LNK_REMOVE`             | 链接器应从最终映像中移除此节 |

---

## 3. `IMAGE_OPTIONAL_HEADER.DllCharacteristics`（DLL 特性标志）

[→ 官方 MSDN 文档](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#dll-characteristics)

### 安全、兼容性、运行时行为控制（重点在现代安全机制）

| 值        | 宏定义                                        | 中文含义                        |
| -------- | ------------------------------------------ | --------------------------- |
| `0x0040` | `IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE`    | 支持 ASLR（地址空间随机化）            |
| `0x0100` | `IMAGE_DLLCHARACTERISTICS_NX_COMPAT`       | 支持 DEP（数据执行保护）              |
| `0x0200` | `IMAGE_DLLCHARACTERISTICS_NO_ISOLATION`    | 禁用侧边加载清单隔离                  |
| `0x0400` | `IMAGE_DLLCHARACTERISTICS_NO_SEH`          | 禁用结构化异常处理                   |
| `0x0800` | `IMAGE_DLLCHARACTERISTICS_NO_BIND`         | 禁用绑定导入                      |
| `0x1000` | `IMAGE_DLLCHARACTERISTICS_APPCONTAINER`    | AppContainer 支持             |
| `0x4000` | `IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA` | 高熵地址支持（ASLR 更安全）            |
| `0x8000` | `IMAGE_DLLCHARACTERISTICS_GUARD_CF`        | 支持控制流保护（Control Flow Guard） |

---

## 举个例子：

你常见的 `DllCharacteristics` 值：`0x8140`

```c
0x8000 -> GUARD_CF（控制流保护）
0x0100 -> NX_COMPAT（DEP）
0x0040 -> DYNAMIC_BASE（ASLR）
```

表示该 DLL 支持 ASLR、DEP 和 CFG，是一个符合现代安全要求的模块。

---

## ✅ 汇总对比表（适合写壳、分析 PE 或制作加载器）：

| 字段                   | 所属结构                    | 描述对象      | 用途                | 示例                      | MSDN 链接                                                                                       |
| -------------------- | ----------------------- | --------- | ----------------- | ----------------------- | --------------------------------------------------------------------------------------------- |
| `Characteristics`    | `IMAGE_FILE_HEADER`     | 整个 PE 文件  | EXE/DLL/是否重定位等    | `0x2002` = 可执行 DLL      | [文档](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_file_header)    |
| `Characteristics`    | `IMAGE_SECTION_HEADER`  | 节（.text等） | 可执行/可读/可写/是否含代码等  | `0x60000020`            | [文档](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-image_section_header) |
| `DllCharacteristics` | `IMAGE_OPTIONAL_HEADER` | 安全机制与兼容性  | 控制 ASLR、DEP、CFG 等 | `0x8140` = ASLR+DEP+CFG | [文档](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#dll-characteristics)     |
