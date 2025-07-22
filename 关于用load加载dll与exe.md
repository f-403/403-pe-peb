

---

# 📘 LoadLibrary 执行 EXE + dll文件无 DllMain 的行为分析

---

## 📚 目录

1. [❌ LoadLibrary 能否执行 EXE 文件？](#loadlibrary-能否执行-exe-文件)

   * [🔴 问题分析](#问题分析)
   * [✅ 正确做法](#正确做法)
2. [📦 DLL 中没有 DllMain 会发生什么？](#dll-中没有-dllmain-会发生什么)

   * [🔍 编译器默认行为](#编译器默认行为)
   * [🧪 EOP 执行行为](#eop-执行行为)
   * [✅ 建议写法](#建议写法)
3. [📌 总结对比表](#总结对比表)
4. [📁 附：EXE 节信息读取示例代码](#附exe-节信息读取示例代码)

---

## ❌ LoadLibrary 能否执行 EXE 文件？

### 🔴 问题分析

* `LoadLibrary` 是为 **DLL 加载** 设计的函数。
* EXE 并不是设计为供其他模块加载执行的，因此：

| 操作                     | 结果                        |
| ---------------------- | ------------------------- |
| `LoadLibrary("1.dll")` | ✅ 正常：加载 DLL 并调用 `DllMain` |
| `LoadLibrary("1.exe")` | ⚠️ 非法或未定义行为，可能失败或返回错误模块句柄 |

* 如果你尝试这样执行 EXE：

```cpp
void* EOP = nt_head->OptionalHeader.AddressOfEntryPoint + (BYTE*)dll;
((void(*)())EOP) (); // 手动跳转入口点
```

会遇到以下问题：

* EXE 的入口是 `WinMainCRTStartup`，它假设已经初始化了命令行参数、堆、句柄等环境；
* `LoadLibrary` 不会建立新的线程或进程上下文；
* 所以你跳转过去，**几乎一定跳飞或崩溃**。

---

### ✅ 正确做法

| 目的            | 建议方案                                          |
| ------------- | --------------------------------------------- |
| 想运行 EXE 主逻辑   | 写一个**手动加载器**：加载文件 → 映射 → 修复 → 执行入口点           |
| 想从 EXE 提取节数据  | 用 `ifstream` 打开 EXE 文件并解析节表                   |
| 想运行 shellcode | 提取 `.text` 段 → `VirtualAlloc` + `memcpy` + 执行 |
| 想运行 DLL 的逻辑   | 正确使用 `LoadLibrary` 加载 DLL 并调用导出或 `DllMain`    |

---

## 📦 DLL 中没有 DllMain 会发生什么？

### 🔍 编译器默认行为

如果你没有显式写 `DllMain`，链接器通常会自动生成一个默认的入口：

```cpp
BOOL WINAPI DllMainCRTStartup(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
    return TRUE; // 什么都不做
}
```

**作用：**

* 避免 PE 文件没有入口而无法加载；
* 允许 DLL 被系统/用户加载但什么都不执行；
* 通常不会调用全局构造函数（除非你使用了 CRT 运行库）。

---

### 🧪 EOP 执行行为

如果你用 `AddressOfEntryPoint` 作为入口执行 DLL：

* 如果你定义了 `DllMain`，就执行它；
* 如果你没写，跳转到的是默认的 `DllMainCRTStartup`；
* 结果通常是“什么都不发生”。

### ✅ 建议写法

若想通过 `EOP` 控制执行：

* 建议你**明确写一个 DllMain**，或者
* 让你的自定义代码从 `.text` 开始运行（即做成 shellcode 风格），否则手动跳转没意义。

---

## 📌 总结对比表

| 场景                     | 有无 DllMain | 行为             | 说明                     |
| ---------------------- | ---------- | -------------- | ---------------------- |
| `LoadLibrary("1.dll")` | 无 DllMain  | ✔️ DLL 会加载     | 但不会执行初始化逻辑             |
| `LoadLibrary("1.exe")` | 无关         | ❌ 不合法或未定义      | EXE 不是 DLL，行为不可预测      |
| 执行 DLL 的 `EOP`         | 无 DllMain  | ✔️ 跳转，但执行默认空入口 | 不会执行你想要的初始化或逻辑         |
| 调用 DLL 的导出函数           | 无 DllMain  | ✔️ 正常调用导出函数    | 不受 `DllMain` 是否存在的影响   |
| 手动加载 DLL → 执行 `EOP`    | 无 DllMain  | ✔️ 跳转默认入口，但无逻辑 | 建议 `.text` 写入逻辑或显式入口定义 |

---

## 📁 附：EXE 节信息读取示例代码

如你只想查看 EXE 的结构和节信息，建议用如下方式解析：

```cpp
#include <iostream>
#include <Windows.h>
#include <fstream>

int main() {
    std::ifstream file("1.EXE", std::ios::binary | std::ios::ate);
    if (!file) return 1;

    size_t size = file.tellg();
    file.seekg(0);

    BYTE* buffer = new BYTE[size];
    file.read((char*)buffer, size);
    file.close();

    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)buffer;
    IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)(buffer + dos_head->e_lfanew);
    IMAGE_SECTION_HEADER* se_head = (IMAGE_SECTION_HEADER*)(buffer + dos_head->e_lfanew +
        sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + nt_head->FileHeader.SizeOfOptionalHeader);

    std::cout << "EXE 节信息：" << std::endl;
    for (int i = 0; i < nt_head->FileHeader.NumberOfSections; i++) {
        BYTE name[9]{};
        memcpy(name, se_head[i].Name, 8);
        std::cout << name << " RVA: 0x" << std::hex << se_head[i].VirtualAddress << std::endl;
    }

    delete[] buffer;
    return 0;
}
```

---
