
---

# 📦 `#pragma code_seg` 与 `#pragma comment(linker, "/SECTION:...")` 用法总结（含读写执行段）

---

## 📌 一、目标用途

- 将 **函数 / 数据 / Shellcode** 放入自定义段
- 设置该段为 **可读、可写、可执行（RWE）**
- 便于提取 Shellcode 或执行自包含代码段

---

## 🧱 二、关键语法说明

| 语法                                                | 功能说明                                           |
|-----------------------------------------------------|----------------------------------------------------|
| `#pragma code_seg("段名")`                          | 更改当前代码段（适用于函数）                      |
| `__declspec(allocate("段名"))`                      | 将变量或数组放入指定段（适用于数据）              |
| `#pragma comment(linker, "/SECTION:段名,RWE")`      | 修改链接器设置，将指定段设置为 可读写执行权限     |

---

## 🧪 三、完整示例：把 Shellcode 放入 `.shellcode` 节，并设置 RWE 权限

```cpp
#include <Windows.h>
#include <iostream>

// 设置链接器参数：让 .shellcode 段可执行、可读、可写
#pragma comment(linker, "/SECTION:.shellcode,RWE")

// 将以下变量放入 .shellcode 节中
#pragma code_seg(".shellcode")
__declspec(allocate(".shellcode"))
char shellcode[] = {
    0x90, 0x90, 0xC3  // NOP, NOP, RET
};
#pragma code_seg()  // 恢复默认代码段

typedef void(*ShellFunc)();

int main() {
    ShellFunc func = (ShellFunc)shellcode;
    func();  // 执行 shellcode
    std::cout << "Shellcode 执行完毕\n";
    return 0;
}
```

---

## ⚙️ 四、说明解读

### 📌 `#pragma comment(linker, "...")`

等价于手动添加到链接器命令行的参数。例如：

```bash
/SECTION:.shellcode,RWE
```

这样可以在不修改 Visual Studio 项目属性的前提下，在源码中完成段权限设置。

---

### 📌 `#pragma code_seg`

适用于函数：

```cpp
#pragma code_seg(".mycode")
void MyFunc() {
    // 此函数会被编译进 .mycode 节
}
#pragma code_seg()
```

---

### 📌 `__declspec(allocate(...))`

适用于变量和数据：

```cpp
__declspec(allocate(".mydata"))
char message[] = "Hello from custom section!";
```

> 📌注意：`__declspec(allocate())` 是必须的，否则变量仍会被编译器分配到默认 `.data` 或 `.rdata`。

---

## 🧷 五、验证段权限是否成功

编译后使用 `dumpbin` 验证：

```bash
dumpbin /headers your_program.exe | findstr .shellcode
```

示例输出：

```
.she­llcode     00000008 readable, writable, executable
```

---

## 📌 六、实用建议

| 操作目标                     | 推荐写法                                                              |
|------------------------------|------------------------------------------------------------------------|
| 把函数放入自定义段           | `#pragma code_seg(".mytext")` + `void func() {...}`                   |
| 把数组放入自定义段           | `__declspec(allocate(".mytext")) char a[] = {...};`                   |
| 设置段权限为RWE              | `#pragma comment(linker, "/SECTION:.mytext,RWE")`                     |
| 多个段分优先顺序             | `.myshell$A`, `.myshell$B`，链接器会按字母排序合并                   |

---

## ✅ 七、总结速查表

| 用途             | 用法                                                              |
|------------------|-------------------------------------------------------------------|
| 自定义段名       | `.shellcode`, `.mytext`, `.payload` 等                            |
| 放入代码段       | `#pragma code_seg(".段名")` + 函数体                               |
| 放入数据段       | `__declspec(allocate(".段名")) char x[] = {...};`                 |
| 设置段权限       | `#pragma comment(linker, "/SECTION:.段名,RWE")`                   |
| 恢复默认段       | `#pragma code_seg()`                                              |

---

