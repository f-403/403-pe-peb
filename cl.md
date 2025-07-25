好的，下面是一个全面的 **使用 `cl.exe`（MSVC 编译器）生成 EXE / DLL 文件并修改入口点与子系统的详细参数文档**，适合底层开发、SHELLCODE载入器、红队等方向使用。

---

# 📘 使用 `cl.exe` 生成 EXE/DLL 的参数详解文档

---

## 📑 目录

1. [编译工具简介（`cl.exe` 和 `link.exe`）](#1-编译工具简介)
2. [生成 EXE 文件（控制台/窗口子系统）](#2-生成-exe-文件)
3. [生成 DLL 文件（默认入口与自定义入口）](#3-生成-dll-文件)
4. [修改程序入口点 `/ENTRY`](#4-修改程序入口点-entry函数名)
5. [修改子系统类型 `/SUBSYSTEM`](#5-修改子系统类型-subsystemtype)
6. [关闭 CRT 依赖 `/NODEFAULTLIB`](#6-关闭-crt依赖-nodefaultlib)
7. [DLL 导出函数（自动 vs `.def` 文件）](#7-dll-导出函数方法)
8. [示例集合（含 EXE/DLL/自定义入口）](#8-示例集合)
9. [常用参数对照表](#9-常用参数对照表)
10. [附录：混合汇编、裸入口、SHELLCODE建议](#10-附录进阶建议)

---

## 1. 编译工具简介

* `cl.exe`：Microsoft C/C++ 编译器
* `link.exe`：Microsoft 链接器，被 `cl` 自动调用
* 推荐打开 `x64 Native Tools Command Prompt for VS` 进行操作

基本语法：

```bash
cl [编译选项] source.cpp [/link [链接选项]]
```

---

## 2. 生成 EXE 文件

### 控制台程序（Console）

入口点：`main()`
子系统：`CONSOLE`（默认）

```bash
cl hello.cpp
# 或指定子系统
cl hello.cpp /link /SUBSYSTEM:CONSOLE
```

### 窗口程序（Windows）

入口点：`WinMain()`
子系统：`WINDOWS`

```bash
cl gui.cpp /link /SUBSYSTEM:WINDOWS
```

---

## 3. 生成 DLL 文件

默认入口点为 `DllMain`，需加 `/LD`：

```bash
cl dllmain.cpp /LD
```

等价于：

```bash
cl dllmain.cpp /link /DLL
```

> 如果要导出函数：使用 `__declspec(dllexport)` 或 `.def` 文件（见第 7 节）

---

## 4. 修改程序入口点 /ENTRY:<函数名>

无论是 EXE 还是 DLL 都可以显式指定入口函数：

```bash
cl myfile.cpp /link /ENTRY:MyCustomEntry
```

> ⚠️ 若你不使用 CRT，请同时加上 `/NODEFAULTLIB`

### 使用自定义入口的注意事项：

* 函数需使用 `extern "C"`（避免 C++ 名字修饰）
* 函数签名必须符合 PE 类型（EXE 通常是 `void WINAPI func()`）

例如：

```cpp
extern "C" void WINAPI MyEntry() {
    MessageBoxA(0, "Hello", "Custom Entry", 0);
}
```

---

## 5. 修改子系统类型 /SUBSYSTEM:<type>

| 子系统类型   | 描述           | 常用入口函数               |
| ------- | ------------ | -------------------- |
| CONSOLE | 控制台程序（默认）    | `main()`             |
| WINDOWS | 窗口程序（无控制台）   | `WinMain()`          |
| NATIVE  | 本地系统服务（NT核心） | `NtProcessStartup()` |

用法示例：

```bash
cl code.cpp /link /SUBSYSTEM:WINDOWS /ENTRY:MyGuiMain
```

---

## 6. 关闭 CRT依赖 /NODEFAULTLIB

如果你写的是最小化的代码（如 SHELLCODE 载入器），不需要 CRT：

```bash
cl loader.cpp /link /ENTRY:MyEntry /NODEFAULTLIB /SUBSYSTEM:CONSOLE
```

> ⚠️ 如果使用 `main()`，不能禁用 CRT，因为 `main()` 不是 EXE 真正入口，CRT 会调用 `main()`。

---

## 7. DLL 导出函数方法

### 方法一：使用 `__declspec(dllexport)`

```cpp
extern "C" __declspec(dllexport) void MyExport() {
    // ...
}
```

### 方法二：使用 `.def` 文件

**my.def**

```def
LIBRARY mydll
EXPORTS
    MyExport
```

编译命令：

```bash
cl dll.cpp /LD /link /DEF:my.def
```

---

## 8. 示例集合

### 控制台 EXE（默认）

```bash
cl main.cpp
```

### 窗口 GUI EXE（指定子系统）

```bash
cl win.cpp /link /SUBSYSTEM:WINDOWS
```

### DLL（含导出）

```bash
cl mydll.cpp /LD
```

### DLL + 自定义入口 + 导出函数

```bash
cl mydll.cpp /LD /link /ENTRY:DllStart /DEF:my.def /NODEFAULTLIB
```

### EXE + 自定义入口 + 无 CRT

```bash
cl raw.cpp /link /ENTRY:MyStart /SUBSYSTEM:CONSOLE /NODEFAULTLIB
```

---

## 9. 常用参数对照表

| 参数                  | 说明                                  |
| ------------------- | ----------------------------------- |
| `/LD`               | 生成 DLL（含 /DLL）                      |
| `/Fe<name>`         | 指定 EXE 输出名                          |
| `/Fo<name>`         | 指定 OBJ 输出名                          |
| `/Tc`               | 强制按 C 编译                            |
| `/TP`               | 强制按 C++ 编译                          |
| `/Ox`               | 启用最大优化                              |
| `/GS-`              | 关闭栈检查（SHELLCODE推荐）                  |
| `/NODEFAULTLIB`     | 不链接 CRT，适合最小 PE                     |
| `/ENTRY:<name>`     | 修改程序入口点                             |
| `/SUBSYSTEM:<type>` | 指定子系统（CONSOLE/WINDOWS/NATIVE）       |
| `/DEF:<file>`       | 指定 .def 文件用于导出函数                    |
| `user32.lib` 等      | 手动链接 Windows API 库，如 MessageBoxA 所需 |

---

## 10. 附录进阶建议

* 若入口点想用裸函数，推荐使用 `__declspec(naked)`
* SHELLCODE 用 `__stdcall` 或 `WINAPI` 保证栈平衡
* 使用 `.asm` 汇编配合 `.cpp` 可创建极简 PE（适合免杀研究）
* 使用 `dumpbin /headers your.exe` 检查入口点、子系统

---
