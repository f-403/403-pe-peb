## 什么是 PEB

PEB（Process Environment Block）是 Windows 进程在用户空间的一个结构，包含了大量与进程运行相关的信息。

> ✅ 它包含：
> 
> * 模块列表（加载了哪些 DLL）
> * 进程参数（命令行、环境变量）
> * 堆地址信息
> * 是否为调试状态

**反病毒软件/静态分析器** 通常不会修改 PEB，所以手动访问它可用于规避检测。

它可以通如下地址获取：

```cpp
#ifdef _WIN64
    PPEB peb = (PPEB)__readgsqword(0x60);
#else
    PPEB peb = (PPEB)__readfsdword(0x30);
#endif
```

或用汇编获取:

```cpp
#ifdef _WIN64
PVOID P = (PVOID)__readgsqword(0x60); // x64: GS:[0x60]，64位不能内嵌汇编
#else
PVOID P;
__asm {
    mov eax, fs:[0x30]  // x86: FS:[0x30]
    mov P, eax
}
#endif
```

---

## PEB 结构简介

```cpp
typedef struct _PEB {
  BYTE                          Reserved1[2];
  BYTE                          BeingDebugged;
  BYTE                          Reserved2[1];
  PVOID                         Reserved3[2];
  PPEB_LDR_DATA                 Ldr; // 模块加载器信息
  // ...
} PEB, *PPEB;
```

---

## PEB_LDR_DATA 结构（模块加载链表）

```cpp
typedef struct _PEB_LDR_DATA {
    ULONG Length;                          // 结构体大小
    BOOLEAN Initialized;                   // 是否初始化
    PVOID SsHandle;                        // 保留字段
    LIST_ENTRY InLoadOrderModuleList;      // 加载顺序的模块链表
    LIST_ENTRY InMemoryOrderModuleList;    // 内存顺序的模块链表
    LIST_ENTRY InInitializationOrderModuleList; // 初始化顺序的模块链表
    PVOID EntryInProgress;                 // 正在加载的模块
    BOOLEAN ShutdownInProgress;           // 是否正在关闭
    PVOID ShutdownThreadId;               // 正在关闭的线程
} PEB_LDR_DATA, *PPEB_LDR_DATA;
```

## LIST_ENTRY 结构（双向链表）

```cpp
typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY* Flink;  // Forward link 下一个节点
    struct _LIST_ENTRY* Blink;  // Backward link 前一个节点
} LIST_ENTRY
```

它们的每一个 Flink/Blink 指向的是 _LDR_DATA_TABLE_ENTRY 结构体中对应的 LIST_ENTRY 字段，例如：

## _LDR_DATA_TABLE_ENTRY结构

```cpp
typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;              // 0x00：加载顺序链表
    LIST_ENTRY InMemoryOrderLinks;            // 0x10：内存顺序链表
    LIST_ENTRY InInitializationOrderLinks;    // 0x20：初始化顺序链表
    PVOID      DllBase;                       // 0x30：模块加载基址（ImageBase）
    PVOID      EntryPoint;                    // 0x38：模块入口点地址
    ULONG      SizeOfImage;                   // 0x40：模块映像大小（整个PE大小）
    UNICODE_STRING FullDllName;               // 0x48：模块全路径（如：C:\Windows\System32\ntdll.dll）
    UNICODE_STRING BaseDllName;               // 0x58：模块名称（如：ntdll.dll）
    // 以下字段可选用于高级用途
    // ULONG Flags;                          // 0x68：模块标志位
    // USHORT LoadCount;                     // 0x6C：加载计数
    // USHORT TlsIndex;                      // ...
    // LIST_ENTRY HashLinks;                // ...
    // PVOID SectionPointer;                // ...
    // ULONG CheckSum;                      // ...
    // 更多字段根据版本不同而扩展
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
//LIST_ENTRY指向这个结构体中相应的字段， 之后向上偏移， 会跳到这个结构中的起始位置， 从而能转化成这个结构体。
```

## LDR_DATA_TABLE_ENTRY里的DllBas指向下面这个结构:
> LDR_DATA_TABLE_ENTRY里有些字段也定义为这个结构

```cpp
typedef struct _UNICODE_STRING {
    USHORT Length;        // 字符串字节长度
    USHORT MaximumLength; // 最大长度（分配空间）
    PWSTR  Buffer;        // 指向宽字符字符串（wchar_t*）
} UNICODE_STRING;
//比如打印dll名字：wprintf(L"模块名称: %ws\n", entry->BaseDllName.Buffer);
```

```text
在 Windows 的 PEB（Process Environment Block）结构中，_PEB_LDR_DATA 中的 LIST_ENTRY 结构中的节点指向 LDR_DATA_TABLE_ENTRY。准确地说是 _LDR_DATA_TABLE_ENTRY 并不直接声明在 _PEB_LDR_DATA 里，而是作为链表中的节点结构使用。

 每个模块节点（LDR_DATA_TABLE_ENTRY）都会嵌入在一个链表中，通过不同的 `LIST_ENTRY` 字段链接起来

每一个加载到当前进程的模块（DLL 或 EXE）对应一个 _LDR_DATA_TABLE_ENTRY。
每加载一个模块（无论是 EXE 还是 DLL），系统就会分配一个 LDR_DATA_TABLE_ENTRY，并将其插入到 PEB_LDR_DATA 中的三个链表中。
这些链表是：
> - InLoadOrderModuleList
> - InMemoryOrderModuleList
> - InInitializationOrderModuleList

因此：
- 一个进程有多个模块
- 每个模块都有一个对应的 LDR_DATA_TABLE_ENTRY 结构
- 所有这些结构通过 LIST_ENTRY 串联形成双向链表
```

```
## 结构关系
```lua
PEB
└── Ldr (PPEB_LDR_DATA)
    ├── InLoadOrderModuleList       --> LIST_ENTRY → 指向 _LDR_DATA_TABLE_ENTRY.InLoadOrderLinks
    ├── InMemoryOrderModuleList     --> LIST_ENTRY → 指向 _LDR_DATA_TABLE_ENTRY.InMemoryOrderLinks
    └── InInitializationOrderModuleList --> LIST_ENTRY → 指向 _LDR_DATA_TABLE_ENTRY.InInitializationOrderLinks

PEB
 └──► PEB_LDR_DATA（peb->Ldr）
       ├──► InLoadOrderModuleList         （LIST_ENTRY） ⎫
       ├──► InMemoryOrderModuleList       （LIST_ENTRY） ─► 多个模块组成的链表，每个节点都指向一个LDR_DATA_TABLE_ENTRY
       └──► InInitializationOrderModuleList（LIST_ENTRY）⎪
                                             ↓
                                 LDR_DATA_TABLE_ENTRY（模块信息结构）
                                  ├── InLoadOrderLinks           （LIST_ENTRY）
                                  ├── InMemoryOrderLinks         （LIST_ENTRY）
                                  └── InInitializationOrderLinks （LIST_ENTRY）
```

## 遍历模块列表（InMemoryOrderModuleList）

```cpp
PPEB_LDR_DATA ldr = peb->Ldr;
LIST_ENTRY* head = &ldr->InMemoryOrderModuleList;
LIST_ENTRY* curr = head->Flink;

while (curr != head) {
    // 直接从 curr 推回 _LDR_DATA_TABLE_ENTRY 的起始地址
    // 这里 InMemoryOrderLinks 偏移为 0x10
    PLDR_DATA_TABLE_ENTRY entry = (PLDR_DATA_TABLE_ENTRY)((BYTE*)curr - 0x10);

    wprintf(L"模块名: %wZ\n", &entry->BaseDllName);

    curr = curr->Flink;
}
```