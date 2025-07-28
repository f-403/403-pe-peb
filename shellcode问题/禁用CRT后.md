✅ **如果你禁用了 CRT（C Runtime Library）**，那么你就**不能再使用大部分 C 标准库函数**，包括但不限于：

---

## ❌ 不能使用的 C 标准库（因为依赖 CRT）

| 类别   | 示例函数                                          | 状态（禁用 CRT 后） |
| ---- | --------------------------------------------- | ------------ |
| IO   | `printf`, `scanf`, `fopen`, `fread`, `fwrite` | ❌ 不可用        |
| 内存   | `malloc`, `free`, `realloc`, `calloc`         | ❌ 不可用        |
| 字符串  | `strlen`, `strcpy`, `strcat`, `strcmp`        | ⚠️ 需要自己实现    |
| 时间   | `time`, `clock`, `localtime`, `strftime`      | ❌ 不可用        |
| 数学   | `sqrt`, `pow`, `sin`, `rand`, `srand`         | ❌ 不可用        |
| 异常处理 | `setjmp`, `longjmp`                           | ❌ 不可用        |

---

## ✅ 可以使用的：**Windows API 里提供的等效功能**

| 需求    | 推荐替代 WinAPI                                 | 说明                     |
| ----- | ------------------------------------------- | ---------------------- |
| 输出    | `WriteConsoleA`, `MessageBoxA`, `WriteFile` | 推荐用                    |
| 内存分配  | `HeapAlloc`, `VirtualAlloc`                 | 不依赖 CRT                |
| 字符串处理 | `lstrlenA`, `lstrcpyA`, `lstrcmpA`          | Windows 提供的一部分 ANSI 函数 |
| 文件操作  | `CreateFile`, `ReadFile`, `WriteFile`       | 推荐                     |
| 时间    | `GetSystemTime`, `GetTickCount`             | 可替代 CRT 时间函数           |

---

## ✅ 示例：用 WinAPI 替代 C 库功能

### 替代 `printf`：

```cpp
void Print(const char* s)
{
    DWORD written;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), s, lstrlenA(s), &written, NULL);
}
```

### 替代 `malloc`：

```cpp
void* my_malloc(SIZE_T size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}
```

---

## ❗ CRT 到底是啥？为什么禁用就不能用 C 函数？

CRT 是 C Runtime，它负责：

* 初始化标准库（`printf`, `malloc`, 全局变量）
* 注册全局构造函数/析构函数
* 设置异常处理栈帧
* 启动入口 `mainCRTStartup → main`

如果你禁用了 CRT，就好比：

> "你剥离了 C 语言赖以生存的基础环境"

因此，`printf()`、`malloc()` 就没法用，除非你自己重写一套。

---

## ✅ 小结

| 是否能用 C 库函数 | 说明                                    |
| ---------- | ------------------------------------- |
| ❌ 不能       | 你禁用了 CRT，所以 `malloc`、`printf` 等函数不可用  |
| ✅ 可以       | 除非你重新链接 CRT（如 `/MT`、`/MD`），或者自己实现类似功能 |

---

## 🛠️ 如果你仍想用少部分 C 函数，有两种折中方式：

### ✅ 方法1：只禁掉部分 CRT（高级技巧）

用 `/NODEFAULTLIB` 排除部分库，再手动链接 `libcmt.lib` 中需要的符号，**极其复杂，不推荐**

### ✅ 方法2：自己实现最常用的函数（轻量方案）

比如：

* 自己写 `my_strlen()` 替代 `strlen`
* 自己写 `itoa`、`memcpy` 等轻量函数
* 用 WinAPI 实现所有需求

---

