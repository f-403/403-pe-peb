// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
void main() {

	MessageBoxA(0, "hello, world", "IAT-重定位修复", 0);
	MessageBoxA(0, "hello, world", "IAT-重定位修复", 0);
	
}
//cl test.cpp /link /ENTRY:_test /NODEFAULTLIB /SUBSYSTEM:CONSOLE
//如果你这样写， 指定入口，但手动加载后， VIRTUALLOC不能指定地址