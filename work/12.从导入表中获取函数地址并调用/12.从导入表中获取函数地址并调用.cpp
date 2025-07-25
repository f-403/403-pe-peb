// 11.导入表-打印dll名字.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
int main()
{
    HMODULE dll = LoadLibraryA("1.dll");

    //转PE结构
    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)dll;
    IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)((BYTE*)dll + dos_head->e_lfanew);
    printf("DOS头标记:%x\n", dos_head->e_magic);
    printf("PE头标记:%x\n", nt_head->Signature);

    IMAGE_DATA_DIRECTORY data = nt_head->OptionalHeader.DataDirectory[1];
    if (data.Size == 0 || data.VirtualAddress == 0) {
        printf("没有导入表\n");
        return 1;
    }
    //获取导入表结构
    IMAGE_IMPORT_DESCRIPTOR* my_import = (IMAGE_IMPORT_DESCRIPTOR*)((data.VirtualAddress) + (BYTE*)dll);

    for (;;) {
        BYTE* dll_name = my_import->Name + (BYTE*)dll;//获取dll的名字
        if (my_import->Name == 0) {
            printf("没有更多导入dll\n");
            break;
        }

        printf("dll名字是:%s\n", dll_name);
        //打印DLL中对应的函数
      
        IMAGE_THUNK_DATA* name_tables = (IMAGE_THUNK_DATA*)( my_import->OriginalFirstThunk + (BYTE*)dll);//导入名称表 INT
        IMAGE_THUNK_DATA* function_tables = (IMAGE_THUNK_DATA*)(my_import->FirstThunk + (BYTE*)dll);//导入地址表 IAT
        //name_tables/function_tables

        for (;;) {
            if (name_tables->u1.AddressOfData == 0) break;
            if (((name_tables->u1.Ordinal) >> 63) & 1) { //我目前程序是64位
                //1010 >> 3 0001 & 1 -> 1
                //0101 >> 3 0000 & 1 -> 0
                printf("函数为序号导入\n");
                //获取函数序号时， 一定要用int,也就是， 要用导入名称表中的Ordinal 
                DWORD function_number = name_tables->u1.Ordinal & 0b1111111111111111;
                printf("当前函数序号为:%d\n", function_number);
            }
            else {
                printf("函数为名称导入\n");
                IMAGE_IMPORT_BY_NAME* function_name = (IMAGE_IMPORT_BY_NAME*)(name_tables->u1.AddressOfData + (BYTE*)dll);//获取名称结构
                printf("函数名称为:%s\n", function_name->Name);
                if (  lstrcmpiA(function_name->Name,"MessageBoxA") == 0  ) {
                    printf("MessageBoxA找到了\n");
                    //获取地址， 一定要用地址导入表: IAT
                    ULONGLONG real_address =  function_tables->u1.Function;//获取函数地址
                    //调用地址
                    ( ( int (*)(HWND, LPCTSTR, LPCSTR, UINT) )real_address ) (0, LPCTSTR("HELLO,WORLD"), LPCSTR("IAT -TITLE"),0);//调用函数

                }

            }
            name_tables++;
            function_tables++;//两个要同步
        }

        my_import++;//跳到下一个dll结构
    }



}
