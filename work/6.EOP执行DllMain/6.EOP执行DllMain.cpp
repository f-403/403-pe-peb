// 6.EOP执行DllMain.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
int main()
{
    HMODULE dll = LoadLibraryA("1.dll");
    if (!dll) {
        std::cout << "导入失败" << std::endl;
        return 0;
    }
    //FreeLibrary(dll);
    //return 0;
    //PE结构
    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)dll;
    //NT头
    IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)(
        (BYTE*)dll+
        dos_head->e_lfanew
        );
    printf("DOS头标记:%x\n", dos_head->e_magic);
    printf("NT头标记:%x\n", nt_head->Signature);
    printf("ImageBase地址:%llx\n",nt_head->OptionalHeader.ImageBase);
    printf("dll载入地址:%p\n", dll);
    DWORD eop_post = nt_head->OptionalHeader.AddressOfEntryPoint;
    printf("eop偏移地址为:%x\n", eop_post);
    //实际执行入口
    void* eop = (BYTE*)dll + eop_post;
    printf("实际函数执行入口:%p\n", eop);
    ((void(*)()) eop) (); //dllmaincrtstartup  
    //dllmaincrtstartup  -> dllmain  

}

//exe / dll
//读入内存
//内存对齐设置好
//获取EOP
// ( void(*)() ) EOP ();

//其实可以执行
//ImageBase ！= 载入地址  --> 要修复重定位表/ IAT表
//ImageBase == 载入地址  --> eop()
//exe EOP
//dll eop -> DllMain