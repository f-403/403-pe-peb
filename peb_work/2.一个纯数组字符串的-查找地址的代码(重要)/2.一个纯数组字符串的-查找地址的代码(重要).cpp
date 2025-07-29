// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#pragma code_seg("shellcode")
#include <iostream>

#include "mypeb.h"


int my_ascii_cmp(const  char* sour, const char* dest);//用于对比字符串
ULONGLONG* my_check_function_address(myLDR_DATA_TABLE_ENTRY* table_entry, ULONGLONG dll_base,  char*);//用于编历PE结构
//函数原型
typedef HMODULE(*myLoadLibraryA)(LPCSTR);
typedef FARPROC(*myGetProcAddress)(HMODULE, LPCSTR);
typedef int(*myMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);


void main()//函数入口可改可不改
{
    
   /* char  load_dll[] = "LoadLibraryA";
    char get_addr[] = "GetProcAddress";
    char messaageboxa_addr[] = "MessageBoxA";
    char dll_name[] = "user32.dll";
    char content[] = "content";
    char title[] = "title";*/
    char load_dll[] = {
    'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0'
    };

    char get_addr[] = {
        'G', 'e', 't', 'P', 'r', 'o', 'c', 'A', 'd', 'd', 'r', 'e', 's', 's', '\0'
    };

    char messaageboxa_addr[] = {
        'M', 'e', 's', 's', 'a', 'g', 'e', 'B', 'o', 'x', 'A', '\0'
    };

    char dll_name[] = {
        'u', 's', 'e', 'r', '3', '2', '.', 'd', 'l', 'l', '\0'
    };

    char content[] = {
        'c', 'o', 'n', 't', 'e', 'n', 't', '\0'
    };

    char title[] = {
        't', 'i', 't', 'l', 'e', '\0'
    };


    myPEB* mypeb = (myPEB*)__readgsqword(0x60);
    PEB_LDR_DATA* ldr = (PEB_LDR_DATA*)mypeb->Ldr;//ldr结构
    LIST_ENTRY* current_list = &ldr->InMemoryOrderModuleList;
    LIST_ENTRY* end_list = current_list->Blink;//获取链表头准备编历
    ULONGLONG* get_load_dll = 0;
    ULONGLONG* get_addr_address = 0;
    ULONGLONG* get_messaageboxa_addr = 0;
    while (current_list != end_list) {
        int offset = (int)&((myLDR_DATA_TABLE_ENTRY*)0)->InMemoryOrderLinks;//偏移量
        //printf("偏移量为:%x\n", offset);
        myLDR_DATA_TABLE_ENTRY* table_entry = (myLDR_DATA_TABLE_ENTRY*)((char*)end_list - offset);
        //wprintf(L"%.*ls\n", (int)table_entry->BaseDllName.Length / sizeof(WCHAR), table_entry->BaseDllName.Buffer);

        //如果找到， 就不再查找
        if (!get_load_dll) {
            get_load_dll = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, load_dll);//返回找到的函数地址
            //if (get_load_dll) return 0;
        }
        if (!get_addr_address) {
            get_addr_address = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, get_addr);//返回找到的函数地址
            //if (get_addr_address) return 0;
        }
        if (!get_messaageboxa_addr) {
            get_messaageboxa_addr = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, messaageboxa_addr);//返回找到的函数地址
            
            //if (get_messaageboxa_addr) return 0;
        }

        end_list = end_list->Blink;//跳到下一个链表
    }
    //int check = my_ascii_cmp("abcdef12f3456", "abcdef12f3456");
    //printf("check:%d\n", check);
    //printf("%p\t%p\t%p\n", get_load_dll, get_addr_address, get_messaageboxa_addr);
    //上面打印，可以看到没有获取到MessageBoxA的地址
    //如要找到了地址， 就load user32.dll做测试
    //以下为测试
    if (get_load_dll && get_addr_address) {
        //当存在两个重要函数时再去获取
        
        myLoadLibraryA test_LoadLibraryA = (myLoadLibraryA)get_load_dll;
        HMODULE test_load = test_LoadLibraryA((dll_name));
        myGetProcAddress test_GetProcAddress = (myGetProcAddress)get_addr_address;
        FARPROC test_box = test_GetProcAddress(test_load, (messaageboxa_addr));
        myMessageBoxA box = (myMessageBoxA)test_box;
        box(NULL, (content), (title), MB_OK);
    }


    //ExitProcess(0);//这个函数没有查找

}
//定义一个函数，用于对比字符串， 只对比IAT表中的函数， 所以直接定义成ASCII对比的
int my_ascii_cmp(const char* sour, const  char* desc) { //区分大小写
    int sour_len = 0;
    for (; sour[sour_len] != '\0'; sour_len++) {
    }
    //printf("sour字符串长度为:%d\n", sour_len);
    int desc_size = 0;
    for (; desc[desc_size] != '\0'; desc_size++) {
    }
    //printf("desc字符串长度为:%d\n", desc_size);
    //#####################################################
    if (sour_len != desc_size) return 0;
    int i = 0;
    for (; sour[i] == desc[i]; i++) {
        if (i == sour_len) break;
    }
    if (sour_len != i) return 0;

    return 1;
}

ULONGLONG* my_check_function_address(myLDR_DATA_TABLE_ENTRY* table_entry, ULONGLONG dll_base,char* check_function_name) {
    //用来查找所有DLL中的两个函数， 一个是LoadLibraryA,另一个是GetProcAddress
    //最后一个为当前进程的exe程序，还没跳过
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)table_entry->DllBase;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((char*)table_entry->DllBase + dos->e_lfanew);

    IMAGE_DATA_DIRECTORY data = nt->OptionalHeader.DataDirectory[0];
    if (data.Size == 0) {
        //printf("当前dll没有导出表\n");
    }
    else {
        //printf("当前dll的导出表大小为:%d\n", data.Size);
        //当存在导出表时， 编历导出表并查找函数地址
        IMAGE_EXPORT_DIRECTORY* my_export = (IMAGE_EXPORT_DIRECTORY*)((char*)dll_base + data.VirtualAddress);
        //char* iat_dll_name = new char[100] {};
        //iat_dll_name = my_export->Name + (char*)dll_base;
        //printf("Dll名字是:%s\n", iat_dll_name);//当前DLL名字
        for (int i = 0; i < my_export->NumberOfNames; i++) {
            DWORD* iat_table = (DWORD*)(my_export->AddressOfNames + (char*)dll_base);
            WORD* iat_ordinals = (WORD*)(my_export->AddressOfNameOrdinals + (char*)dll_base);
            DWORD* iat_address = (DWORD*)(my_export->AddressOfFunctions + (char*)dll_base);
            //
            
            char *iat_function_name = (char*)(iat_table[i] + (char*)dll_base);
            //printf("%d函数名字是:[%s]\n",i, iat_function_name);
            //printf("要查找的函数是:[%s]\n", check_function_name);
            if (my_ascii_cmp(check_function_name, iat_function_name)) {
                //printf("找到函数了-----------------------------------%s\n", check_function_name);
                DWORD ordinals = iat_ordinals[i];//获取对应下标
                ULONGLONG* function_address = (ULONGLONG*)(iat_address[ordinals] + (char*)dll_base);//偏移求得地址
                return function_address;
            }
        }
    }
    return 0;
}
    