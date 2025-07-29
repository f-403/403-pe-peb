# 用 #pragma section与#pragma code_seg/data_seg都能创建节

## 用section修改read, write, execute时， 不能修改为write, 要修改vs配置文件， 但用 #pragma comment(linker, "/section:.mycode,RWE")能修改成功
## 用section创建节
```cpp
#pragma comment(linker, "/section:.mycode,RWE")
#pragma section(".mycode", read,write, execute)
__declspec(allocate(".mycode")) char dummy = 0;

int main()
{
    std::cout << "Hello World!\n";
    return 0;
}
```
## 用code_seg/data_seg创建节
```cpp
#pragma comment(linker, "/section:.mycode,RWE")
#pragma code_seg(".mycode") //代码节
//#pragma data_seg(".mycode") //数据节
#pragma section(".mycode", read,write, execute)
__declspec(allocate(".mycode")) char dummy = 0;

int main()
{
    std::cout << "Hello World!\n";
    return 0;
}
```

## 用linker修改生成的PE文件的节属性
```cpp
#pragma comment(linker, "/section:.text,RWE")
#pragma code_seg(".text")

__declspec(allocate(".text")) char dummy = 0;

int main()
{
    std::cout << "Hello World!\n";
    return 0;
}
```

## 用section创建节时能同时指定权限， 但指定不了write
## 如果section/data_seg/code_seg指定的节存在， 就相当于向那个节添加数据


# 注意， 节要有数据在里面的时候才能查到有这个节。用vs要点`重新生成`， 不要直接点`本地调试`， 要不看不到效果。




