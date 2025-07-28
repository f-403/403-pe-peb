## 
---


- `.text`、`.data`、`.rdata`、`.bss` 等段的作用
- 不同声明方式（如 `char name[] = {...}` vs `char *name = "..."`）的存储位置
- 静态变量、全局变量、常量字符串等的具体段落

---

## 🧩 一、PE 各段用途总览

| 段名       | 用途说明                             | 典型内容                              |
|------------|--------------------------------------|----------------------------------------|
| `.text`    | 代码段（Code Segment）               | 函数、指令、shellcode、函数地址等      |
| `.data`    | 已初始化的可读写数据                 | 全局变量、静态变量、有初值             |
| `.rdata`   | 只读数据段（Read-only Data）         | 字符串常量、`const` 常量、vftable 等   |
| `.bss`     | 未初始化数据（由 loader 清零）       | 无初值的全局变量、静态变量             |
| `.idata`   | 导入表段（Import Address Table）     | DLL 导入信息，API 函数地址             |
| `.edata`   | 导出表段                             | 本模块导出函数地址                     |
| `.reloc`   | 重定位信息                           | 可重定位地址表                         |

---

## 🧪 二、各种变量的存储位置表

| 变量类型                     | 示例代码                                      | 存储段   | 原因解释 |
|------------------------------|-----------------------------------------------|----------|----------|
| 全局变量（有初值）          | `int g = 5;`                                  | `.data`  | 有初始值，非 const，可写 |
| 全局变量（无初值）          | `int g;`                                      | `.bss`   | 无初值，默认清零 |
| `static` 变量（有初值）     | `static int a = 1;`                           | `.data`  | 本地作用域，但存储方式同全局 |
| `static` 变量（无初值）     | `static int a;`                               | `.bss`   | 同样清零处理 |
| `const` 全局常量            | `const int c = 3;`                            | `.rdata` | 只读段 |
| 字符串数组（值）            | `char str[] = "abc";`                         | `.data`  | 是数组，存值，可写 |
| 字符串指针（指向常量池）    | `char *str = "abc";`                          | `.data` 指针；`.rdata` 字符串 | 指针在 `.data`，字符串文字存在 `.rdata` |
| `const char*` 字符串指针    | `const char* str = "abc";`                    | `.data` 指针；`.rdata` 字符串 | 同上 |
| 字符串常量（隐式）          | `"abc"`                                       | `.rdata` | 编译器放在常量池中 |
| 函数/Shellcode              | `void func() {}` 或 Shellcode 字节流           | `.text`  | 函数体和指令代码 |
| `static const` 常量         | `static const int a = 100;`                   | `.rdata` | const 类变量不可写 |
| `const char[]` 常量数组     | `const char s[] = "abc";`                     | `.rdata` | 是数组但 const，放到只读段 |

---

## 🔍 三、典型变量对比举例

### 1. `char str1[] = "hello";`

- 实际上是数组，存储方式为：`h`,`e`,`l`,`l`,`o`,`\0`
- 占用 6 字节
- **存储在 `.data` 区**，可修改内容

### 2. `char *str2 = "hello";`

- 是一个指针变量，指向 `.rdata` 区域的 `"hello\0"`
- `str2` 本身放在 `.data` 区
- `"hello"` 内容放在 `.rdata`（只读常量池）

### 3. `const char* str3 = "hello";`

- 指针变量不可修改指向内容
- 本质布局与 `char*` 相同：`.data` 指针 + `.rdata` 内容

### 4. `static int g = 123;`

- 是静态局部变量
- 在 `.data` 区，作用域仅限定义它的函数，但生命周期为全局

---

## 📌 四、总结口诀

> 🧠 **段位口诀：**

```
.text 放代码，Shellcode也爱住；
.data 存变量，写得动它；
.bss 是空白，没初值的家；
.rdata 是常量，字符串的家；
```

---

