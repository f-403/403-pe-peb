// 8.def文件导出dll中的函数.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
void test();//带名字
void _my_print();//序号
void _hidd();//无名字
int main()
{
    std::cout << "Hello World!\n";
}
//带序号
//没有名字
void test() {
    printf("这是test()函数\n");
}
void _my_print() {
    printf("这是_my_print()函数\n");
}
void _hidd() {
    printf("这是_hidd()函数\n");
}
