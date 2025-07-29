// 一个手动计算字符串与结构体偏移量的代码.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
struct test {
    int a;
    float b;
};
int my_wstring_check(const WCHAR* so, const WCHAR* des, int des_size);
int main()
{
    //计算长度
    const WCHAR* name = L"ABCD";
    int len = 0;
    while (name[len]) {
        wprintf(L"%lc",name[len]);
        len++;
        
    }
    //判断是否相等
    const WCHAR* swap_name = L"ABDe";
    while (*name && (*name == *swap_name)) {
        wprintf(L"name:%lc\n", *name);
        wprintf(L"swap_name:%lc\n",*swap_name);
        name++;
        swap_name++;
    }
    //先把0转为结构指针
    //((test*)0)
    //再取里面的值
    //((test*)0)->b
    //再把地址转为数字即可
    //   int(   &(((test*)0)->b)   );
    printf("结构中的b变量偏移量为:%d\n", int(&(((test*)0)->b)));
    printf("对比值为:%d\n",my_wstring_check(L"ABCD", L"ABCD", 4));

}

int my_wstring_check(const WCHAR* so, const WCHAR* des, int des_size) {
    //先查看长度
    int len = 0;

    while (so[len]) {
        wprintf(L"字符是:%lc\n", so[len]);
        len++;
    }
    printf("so字符串长度为:%d\n", len);
    if (len != des_size) {
        return 0;
    }
    //如果长度相等
    int check_post = 0;
    while (so[check_post] == des[check_post]) {
        check_post++;
        if (check_post == len) break;
    }
    if (check_post == len) {
        return 1;
    }
    return 0;
}
