# PE 手动加载与 LoadLibrary：重定位与 IAT 修复机制详解

---

## 📑 目录

1. [基础概念](#基础概念)
2. [手动加载 vs LoadLibrary 加载对比](#手动加载-vs-loadlibrary-加载对比)
3. [具体场景说明](#具体场景说明)
   - [LoadLibrary 加载](#loadlibrary-加载)
   - [手动加载](#手动加载)
4. [典型误区澄清](#典型误区澄清)
5. [验证方法建议](#验证方法建议)

---

## 📘 基础概念

- **基地址（ImageBase）**：
  - PE 文件在内存中**期望加载**的位置，定义在 `OptionalHeader.ImageBase` 字段中。

- **实际加载地址**：
  - 实际加载进内存的位置，由你自己手动分配（手动加载）或系统自动分配（LoadLibrary）。

- **重定位表（Relocation Table）**：
  - 若实际加载地址 ≠ ImageBase，需修复 `.text`、`.data` 等段中硬编码了绝对地址的位置。

- **IAT（Import Address Table）**：
  - 指向导入函数的地址，在加载时必须填充。不论是否需要重定位，IAT **都必须修复**。

---

## ⚔️ 手动加载 vs LoadLibrary 加载对比

| 加载方式        | 是否需修复重定位       | 是否需修复 IAT        | 说明                                                         |
|-----------------|------------------------|------------------------|--------------------------------------------------------------|
| LoadLibrary     | 系统自动修复           | 系统自动修复           | 操作系统会自动修复重定位和解析导入表。                       |
| 手动加载        | 若地址不同 ⇒ 需要修复  | 一律需要修复           | 手动映射需要你自己修复导入表和（如有）重定位。              |

---

## 🔍 具体场景说明

### ✅ LoadLibrary 加载

- 系统会自动完成：
  - 读取重定位表并修复；
  - 解析 Import Table 并通过 `GetProcAddress` 填充 IAT。

- **结论：你无需自己处理，系统帮你修好了。**

---

### 🛠️ 手动加载

- 若你把 DLL 映射到和 ImageBase 一致的位置：
  - 可以跳过重定位处理；
  - 但 **IAT 一定要修复**！

- 若加载地址 ≠ ImageBase：
  - 必须修复重定位；
  - IAT 也必须修复。

- **IAT 修复流程大致如下：**
  1. 遍历 `IMAGE_IMPORT_DESCRIPTOR`；
  2. 获取模块名并调用 `LoadLibrary`；
  3. 遍历 Thunk 并调用 `GetProcAddress`；
  4. 将地址写入 IAT。

---

## ❗ 典型误区澄清

| 错误认知                                       | 正确认知                                               |
|----------------------------------------------|--------------------------------------------------------|
| “IAT 只有地址不相等才修复”                   | ❌ 错！IAT 总是要修复，导入表必须自己处理。             |
| “重定位必须修复”                              | ❌ 不一定，若你加载在 ImageBase 上可跳过。              |
| “LoadLibrary 加载就没修复动作”               | ❌ 错！操作系统默默完成了所有重定位和导入表处理。        |

---

## 🧪 验证方法建议

你可以编写 DLL + 三种加载方式来验证上面内容：

1. **用 LoadLibrary 加载**  
   - 自动修复，无需你处理，正常运行。

2. **手动加载到与 ImageBase 相同地址**  
   - 不修重定位，但修复 IAT ⇒ 正常运行。

3. **手动加载到不同地址，且不修重定位**  
   - 程序崩溃或跳转错误，说明重定位是必须的。

---

当然，以下是将上面关于“为什么 `int main(){return 123456;}` 的 EXE 手动加载仍然失败”的详细解释整理成了一份结构清晰的 Markdown 文档，适合用于学习、整理或发布教程。

---



# 那为什么一个看似简单的 `int main(){return 123456;}` 的 EXE 手动加载会失败？


## 📌 问题现象概述

你写了一个最简单的程序：

```cpp
int main() {
    return 123456;
}
````

用手动方式加载此 EXE（如内存映射、非系统加载器等）时，就算`ImageBase==手动加载地址`时，发现程序运行失败或崩溃。为什么？

---

## 🧩 表面代码与实际生成的差异

虽然你代码中没有调用任何函数，但 **编译器生成的最终 EXE 文件仍然包含许多依赖**，包括：

* 运行时初始化代码（CRT）
* 隐式调用如 `ExitProcess`、`GetCommandLineW` 等 API
* 默认入口点为 `mainCRTStartup`，而不是你写的 `main`
* 导入表（IAT）
* 可能存在的 `.reloc`（重定位）表
* 可能存在 TLS 表等

---

## ⚙️ MSVC 默认生成行为

MSVC 编译器会自动：

* 链接到 CRT（C Runtime）
* 设置入口点为 `mainCRTStartup` 或 `WinMainCRTStartup`
* 添加对 `kernel32.dll`、`ucrtbase.dll` 等的导入
* 调用一套完整初始化流程：

  1. 初始化全局构造函数
  2. 设置堆栈
  3. 调用 `main`
  4. 调用 `exit` 或 `ExitProcess`

---

## ❌ 失败的真正原因

| 问题点           | 是否存在 | 是否你处理了？ | 导致后果                  |
| ------------- | ---- | ------- | --------------------- |
| Import Table  | 有    | ❌ 未修复   | IAT 无效，导致函数调用崩溃       |
| 重定位表 `.reloc` | 可能有  | ❌ 未修复   | 加载地址 ≠ ImageBase ⇒ 崩溃 |
| TLS 初始化       | 有时有  | ❌ 未修复   | 多线程相关初始化失败            |
| main 并非真正入口点  | 是    | ❌ 跳转错误  | 根本没进 `main` 就崩溃       |

---
> main函数并不是真正的程序入口

## 🛠️ 解决方案

### ✅ 方式一：生成真正“干净”的 EXE

使用如下代码：

```cpp
// test.cpp
int _start() {
    return 123456;
}
```

编译命令：

```sh
cl test.cpp /nologo /link /NODEFAULTLIB /ENTRY:_start /SUBSYSTEM:CONSOLE
```

这样生成的 EXE：

* 没有链接 CRT
* 没有导入表
* 没有 `.reloc` 表
* 入口点就是你指定的 `_start`
* **适合用于手动加载、Shellcode 包装等场景**
> 实际使用中， 当我指定入口点编译时， 并不能加载到指定地址， 当默认带`main`函数生成exe时,
> 却能加载到我选译的地址。
---

### ✅ 方式二：编写支持修复的 Loader

手动修复加载的关键部分包括：

1. **修复导入表（IAT）**

   * 遍历 `IMAGE_IMPORT_DESCRIPTOR`
   * 使用 `LoadLibraryA` 加载模块
   * 使用 `GetProcAddress` 获取函数地址
   * 写入到 IAT 中

2. **重定位处理（如有）**

   * 检查是否加载地址 ≠ ImageBase
   * 遍历 `.reloc` 表，修复偏移

3. **定位入口点执行**

   * `EntryPoint = ImageBase + OptionalHeader.AddressOfEntryPoint`
   * 创建线程或直接调用

4. **可选：TLS 修复**

   * 一般用于更高级场景，如解密器、加壳

---

## 🔍 如何验证 EXE 结构

你可以用以下工具检查你的 EXE 文件：

* [PE-bear](https://github.com/hasherezade/pe-bear)
* [CFF Explorer](https://ntcore.com/?page_id=388)
* PEview、IDA Free

重点查看：

* `.idata` 节 ⇒ 是否有导入
* `.reloc` 节 ⇒ 是否存在
* EntryPoint 地址
* `IMAGE_IMPORT_DESCRIPTOR` 指向的 DLL 和函数
* 是否存在 TLS 表

---

## 📋 总结表格

| 项目            | 默认 MSVC EXE 有吗？ | 你需要处理吗（手动加载时）      |
| ------------- | --------------- | ------------------ |
| IAT（导入表）      | ✅ 有             | ✅ 必须修复             |
| 重定位表 `.reloc` | 可能有             | ✅ 如果地址 ≠ ImageBase |
| main 函数入口     | ❌ 不是主入口         | ✅ 跳转到实际入口          |
| CRT 初始化       | ✅ 有             | ✅ 除非完全剥离 CRT       |
| TLS（线程局部存储）   | 有可能             | ⚠️ 高级场景需支持         |

---

