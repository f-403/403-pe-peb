非常好！下面我给你两个经典小例子：

---

# ✅ 示例 1：遍历 PEB 中的模块，查找特定模块名（比如 "user32.dll"）

我们通过 `PEB -> Ldr -> InMemoryOrderModuleList` 遍历模块，查找某个模块的 `DllBase`。

```cpp
#include <windows.h>
#include <winternl.h>
#include <stdio.h>

#pragma comment(lib, "ntdll.lib")

typedef struct _PEB_LDR_DATA {
    ULONG Length;
    BOOLEAN Initialized;
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

#ifdef _M_IX86
#define PPEB __readfsdword(0x30)
#elif _M_X64
#define PPEB __readgsqword(0x60)
#endif

int wmain() {
    PPEB peb = (PPEB)PPEB;
    PPEB_LDR_DATA ldr = (PPEB_LDR_DATA)(peb->Ldr);

    LIST_ENTRY* head = &ldr->InMemoryOrderModuleList;
    LIST_ENTRY* curr = head->Flink;

    while (curr != head) {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(curr, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (_wcsicmp(entry->BaseDllName.Buffer, L"user32.dll") == 0) {
            wprintf(L"找到 user32.dll！地址: %p\n", entry->DllBase);
            break;
        }

        curr = curr->Flink;
    }

    return 0;
}
```

---

# ✅ 示例 2：遍历模块 → 找到 user32.dll → 手动查找 `MessageBoxW` → 调用它

```cpp
#include <windows.h>
#include <winternl.h>
#include <stdio.h>

typedef int (WINAPI* MSGBOXW)(HWND, LPCWSTR, LPCWSTR, UINT);

// 上面一样的 LDR_DATA_TABLE_ENTRY 和 PEB 宏定义复用
// ...

int wmain() {
    PPEB peb = (PPEB)__readgsqword(0x60);
    PPEB_LDR_DATA ldr = (PPEB_LDR_DATA)(peb->Ldr);

    LIST_ENTRY* head = &ldr->InMemoryOrderModuleList;
    LIST_ENTRY* curr = head->Flink;

    HMODULE user32Base = NULL;

    while (curr != head) {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(curr, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (_wcsicmp(entry->BaseDllName.Buffer, L"user32.dll") == 0) {
            user32Base = (HMODULE)entry->DllBase;
            break;
        }

        curr = curr->Flink;
    }

    if (user32Base) {
        // 自己实现 GetProcAddress
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)user32Base;
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)user32Base + dos->e_lfanew);

        DWORD exportRVA = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        IMAGE_EXPORT_DIRECTORY* expDir = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)user32Base + exportRVA);

        DWORD* funcNames = (DWORD*)((BYTE*)user32Base + expDir->AddressOfNames);
        WORD* nameOrdinals = (WORD*)((BYTE*)user32Base + expDir->AddressOfNameOrdinals);
        DWORD* functions = (DWORD*)((BYTE*)user32Base + expDir->AddressOfFunctions);

        for (DWORD i = 0; i < expDir->NumberOfNames; ++i) {
            const char* name = (char*)user32Base + funcNames[i];
            if (strcmp(name, "MessageBoxW") == 0) {
                WORD ordinal = nameOrdinals[i];
                DWORD funcRVA = functions[ordinal];
                MSGBOXW pMsgBox = (MSGBOXW)((BYTE*)user32Base + funcRVA);

                pMsgBox(NULL, L"Hello from manual GetProcAddress!", L"Test", MB_OK);
                break;
            }
        }
    } else {
        printf("user32.dll 未找到！\n");
    }

    return 0;
}
```

---

## ✅ 两个示例总结：

| 示例 | 内容 |
|------|------|
| 示例 1 | 遍历 PEB 的模块链表，找到模块名 |
| 示例 2 | 遍历模块 → 查找函数 → 手动调用 `MessageBoxW` |

---

需要我进一步讲解导出表结构或做一个“任意 DLL + 函数名”查找的封装函数也可以告诉我。

当然有！基于你正在深入理解 PEB、LDR、导出表等结构，这里我为你精心整理了一些非常实用的例子，**都是红队、加载器、反检测、手动加载器**等场景常用的技巧。

---

# ✅ 实用例子目录（基于 PEB/LDR/导出表）：

| 📌 例子编号 | 功能                                        | 简要说明                            |
| ------- | ----------------------------------------- | ------------------------------- |
| 1️⃣     | 遍历模块获取任意 DLL 的 Base                       | 比如找 ntdll.dll 的基地址              |
| 2️⃣     | 模拟 GetProcAddress（无API）                   | 不调用系统 API，手动解析导出表               |
| 3️⃣     | 判断是否存在某模块（反注入）                            | 检测黑名单 DLL 是否存在（如 "dbghelp.dll"） |
| 4️⃣     | 获取主模块路径                                   | 即当前 EXE 的完整路径（来自 FullDllName）   |
| 5️⃣     | 使用 LDR 中顺序加载 DLL                          | 实现动态加载但避开 LoadLibraryA 监控       |
| 6️⃣     | 枚举所有加载模块 + EntryPoint                     | 有时你想知道每个模块的入口点地址                |
| 7️⃣     | 获取 kernel32.dll → GetProcAddress → 加载其他模块 | 一般用于 shellcode 中构建 import 表     |
| 8️⃣     | 检测是否被注入（模块链条完整性）                          | 判断链表是否被钩子或断链                    |
| 9️⃣     | Dump 所有模块名到文件                             | 类似“用户层无驱DLL枚举工具”                |
| 🔟      | 判断模块是否被 Unlink（断链）                        | 有些注入技术会 Unlink 自己，反查用           |

---

## 🔧 示例 1：查找 kernel32.dll 的 Base（用于绕过导入表）

```cpp
HMODULE FindModuleBase(const wchar_t* module_name) {
    PPEB peb = (PPEB)__readgsqword(0x60);
    PPEB_LDR_DATA ldr = peb->Ldr;
    LIST_ENTRY* head = &ldr->InLoadOrderModuleList;
    LIST_ENTRY* curr = head->Flink;

    while (curr != head) {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(curr, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if (_wcsicmp(entry->BaseDllName.Buffer, module_name) == 0) {
            return (HMODULE)entry->DllBase;
        }
        curr = curr->Flink;
    }

    return NULL;
}
```

调用方式：

```cpp
HMODULE base = FindModuleBase(L"kernel32.dll");
wprintf(L"kernel32.dll 基地址: %p\n", base);
```

---

## 🔧 示例 2：判断是否存在调试模块（反注入检测）

```cpp
bool IsModuleLoaded(const wchar_t* dll_name) {
    return FindModuleBase(dll_name) != NULL;
}

// 使用示例
if (IsModuleLoaded(L"dbghelp.dll") || IsModuleLoaded(L"ollydbg.dll")) {
    MessageBoxW(NULL, L"检测到调试模块！", L"警告", MB_ICONERROR);
}
```

---

## 🔧 示例 3：获取当前主模块完整路径（不调用 GetModuleFileName）

```cpp
void PrintSelfModulePath() {
    PPEB peb = (PPEB)__readgsqword(0x60);
    PPEB_LDR_DATA ldr = peb->Ldr;
    PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(
        ldr->InLoadOrderModuleList.Flink,
        LDR_DATA_TABLE_ENTRY,
        InLoadOrderLinks
    );

    wprintf(L"当前主模块路径: %wZ\n", &entry->FullDllName);
}
```

---

## 🔧 示例 4：遍历导出表，构建自定义 GetProcAddress（无API）

和之前类似，但封装成函数：

```cpp
FARPROC MyGetProcAddress(HMODULE mod, const char* func_name) {
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)mod;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)mod + dos->e_lfanew);
    IMAGE_EXPORT_DIRECTORY* exp = (IMAGE_EXPORT_DIRECTORY*)((BYTE*)mod +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    DWORD* names = (DWORD*)((BYTE*)mod + exp->AddressOfNames);
    WORD* ordinals = (WORD*)((BYTE*)mod + exp->AddressOfNameOrdinals);
    DWORD* funcs = (DWORD*)((BYTE*)mod + exp->AddressOfFunctions);

    for (DWORD i = 0; i < exp->NumberOfNames; ++i) {
        const char* name = (char*)mod + names[i];
        if (strcmp(name, func_name) == 0) {
            WORD ordinal = ordinals[i];
            return (FARPROC)((BYTE*)mod + funcs[ordinal]);
        }
    }

    return NULL;
}
```

---

## 🔧 示例 5：绕过 IAT 手动执行 LoadLibraryA + GetProcAddress

```cpp
HMODULE k32 = FindModuleBase(L"kernel32.dll");
FARPROC pLoadLibraryA = MyGetProcAddress(k32, "LoadLibraryA");
FARPROC pGetProcAddress = MyGetProcAddress(k32, "GetProcAddress");

// 转换类型并调用
typedef HMODULE(WINAPI* LOADLIBA)(LPCSTR);
typedef FARPROC(WINAPI* GETPROCA)(HMODULE, LPCSTR);

HMODULE user32 = ((LOADLIBA)pLoadLibraryA)("user32.dll");
FARPROC pMsgBox = ((GETPROCA)pGetProcAddress)(user32, "MessageBoxA");

typedef int (WINAPI* MSGBOXA)(HWND, LPCSTR, LPCSTR, UINT);
((MSGBOXA)pMsgBox)(NULL, "绕过 IAT 调用成功", "测试", MB_OK);
```

---

## ✅ 想法延伸：

你可以用这些技巧做：

* **Shellcode loader**（无 IAT，纯手动加载）
* **反调试、反注入模块检测**
* **注入后调用系统模块（user32.dll）里的函数**
* **内存中枚举模块（用于跨进程注入模块）**
* **做一个小型PE解析器或加载器**

---
