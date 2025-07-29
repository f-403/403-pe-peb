

---

# DEF 文件详解及使用说明

## 1. 什么是 DEF 文件？

DEF（Module-Definition File，模块定义文件）是一种文本文件，用于告诉链接器如何导出 DLL 中的函数和符号。它主要用于：

* 明确导出哪些函数（导出表）
* 指定导出函数的序号（Ordinal）
* 控制导出函数是否带有名称（NONAME）
* 指定导出函数别名（别名导出）

DEF 文件是传统的 Windows DLL 导出方法之一，适合精细控制导出函数的细节。

---

## 2. DEF 文件的基本结构

```def
LIBRARY 模块名
DESCRIPTION '模块描述信息'  ; 可选
EXPORTS
    函数名1 @序号1 NONAME
    函数名2 @序号2
    函数名3 = 内部符号名
    函数名4 DATA
```

### 常用关键字说明：

* `LIBRARY`：指定 DLL 的模块名。
* `DESCRIPTION`：对 DLL 的简短描述（可选）。
* `EXPORTS`：导出函数列表。
* `@序号`：指定该函数的导出序号（Ordinal）。
* `NONAME`：导出时不带函数名，仅导出序号，减少导出表体积，提高加载效率。
* `DATA`：标明导出的是数据而非函数。
* `函数名 = 内部符号名`：用外部导出名重命名内部符号。

---

## 3. DEF 文件示例

```def
LIBRARY MyDll
DESCRIPTION '示例DLL模块'

EXPORTS
    PrintMessage @1 NONAME
    AddNumbers @2
    InternalFunc = Real_InternalFunc
```

说明：

* `PrintMessage` 导出时只用序号 `1`，无名称。
* `AddNumbers` 以名称和序号 `2` 导出。
* `InternalFunc` 是外部导出名，内部实际符号名是 `Real_InternalFunc`。

---

## 4. Visual Studio 中使用 DEF 文件的设置

在 Visual Studio 中，使用 DEF 文件通常需要以下步骤：

1. **将 DEF 文件添加到项目**
   把 `.def` 文件添加到项目中（右键项目 > 添加 > 现有项）。

2. **配置链接器使用 DEF 文件**

   * 打开项目属性页（右键项目 > 属性）。
   * 选择 `链接器` > `输入`。
   * 在 `模块定义文件`（Module Definition File）项中填写 DEF 文件名（含路径），例如 `MyDll.def`。

3. **导出符号**

   * 确保代码中函数使用 `__declspec(dllexport)` 或仅通过 DEF 文件导出。

4. **编译链接**

   * 编译时链接器会读取 DEF 文件，按照其中定义导出符号。

---

## 5. `__declspec(dllexport)` 与 DEF 文件的冲突问题

* **`__declspec(dllexport)` 是 MSVC 推荐的现代导出方式**，通过在代码中声明导出，自动生成导出表。

* **当同时使用 DEF 文件和 `__declspec(dllexport)` 时，链接器行为如下：**

  * 如果 DEF 文件中指定了导出，链接器通常会优先使用 DEF 文件控制的导出表。
  * 但是，如果函数使用了 `__declspec(dllexport)`，则该函数的符号会默认带名称导出，且会覆盖 DEF 文件中关于 `NONAME` 的配置。
  * **换句话说：如果函数用 `__declspec(dllexport)` 标记，即使 DEF 文件写了 `NONAME`，该函数仍会以名称导出，DEF 文件中的 `NONAME` 失效。**

---

## 6. 实际问题示例

```cpp
// MyDll.cpp
extern "C" {
    __declspec(dllexport) void test_func();
}
```

对应 DEF 文件：

```def
LIBRARY MyDll
EXPORTS
    test_func @1 NONAME
```

**结果：**

* 运行 `dumpbin /exports`，`test_func` 会带名字导出，`NONAME` 不生效。
* 因为 `__declspec(dllexport)` 会让 MSVC 链接器强制带名导出，覆盖 DEF 的 `NONAME`。

---

## 7. 解决方法建议

* **若想完全用 DEF 文件控制导出（包括 `NONAME`），应避免在代码中使用 `__declspec(dllexport)`，改用纯 DEF 文件导出。**

* **或者仅用 `__declspec(dllexport)`，不要写 DEF 文件，使用 MSVC 自动生成导出表（不支持 `NONAME`）。**

* **两者混用时，`NONAME` 选项在 DEF 中一般无效。**

---

## 8. 总结与推荐

| 方案                        | 优点                       | 缺点                    |
| ------------------------- | ------------------------ | --------------------- |
| 纯 DEF 文件导出                | 精细控制导出序号和名称（支持 `NONAME`） | 需要维护 DEF 文件，较繁琐       |
| 纯 `__declspec(dllexport)` | 代码中控制，方便，自动维护导出表         | 不支持 `NONAME`，导出表会带名字  |
| 混合使用 DEF + `dllexport`    | 理论上兼容，但 `NONAME` 不生效     | 可能引起导出符号冲突或无效的 DEF 配置 |

---

# 附录：DEF 文件常见命令总结

| 关键字           | 作用          | 备注           |
| ------------- | ----------- | ------------ |
| `LIBRARY`     | 指定DLL名称     | 必填           |
| `DESCRIPTION` | DLL的描述信息    | 可选           |
| `EXPORTS`     | 导出符号列表      | 必填           |
| `@序号`         | 指定导出函数序号    | 可提高加载效率      |
| `NONAME`      | 只导出序号，不导出名称 | 节省导出表空间      |
| `DATA`        | 指示导出的是数据符号  | 否则默认是函数      |
| `符号1 = 符号2`   | 重命名导出符号     | 将内部符号2导出为符号1 |

---
