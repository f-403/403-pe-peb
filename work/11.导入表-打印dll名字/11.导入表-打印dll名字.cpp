// 11.导入表-打印dll名字.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
int main()
{
    HMODULE dll = LoadLibraryA("1.dll");

    //转PE结构
    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)dll;
    IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)((BYTE*)dll+dos_head->e_lfanew);
    printf("DOS头标记:%x\n", dos_head->e_magic);
    printf("PE头标记:%x\n", nt_head->Signature);

    IMAGE_DATA_DIRECTORY data = nt_head->OptionalHeader.DataDirectory[1];
    if (data.Size == 0 || data.VirtualAddress == 0) {
        printf("没有导入表\n");
        return 1;
    }
    //获取导入表结构
    IMAGE_IMPORT_DESCRIPTOR* my_import = (IMAGE_IMPORT_DESCRIPTOR*) ((data.VirtualAddress) + (BYTE*)dll);

    for (;;) {
        BYTE* dll_name = my_import->Name + (BYTE*)dll;//获取dll的名字
        if (my_import->Name == 0) {
            printf("没有更多导入dll\n");
            break;
        }
        printf("dll名字是:%s\n", dll_name);
        IMAGE_THUNK_DATA* import_name_table = (IMAGE_THUNK_DATA*)(my_import->OriginalFirstThunk + (BYTE*)dll);//导入名称表
        IMAGE_THUNK_DATA* import_address_table = (IMAGE_THUNK_DATA*)(my_import->FirstThunk + (BYTE*)dll);//导入序号表
        for (;;) {
            if (import_address_table->u1.Function == 0) break;

            //判断是名称导入还是序号导入
            if ((import_address_table->u1.Ordinal) >> 63 & 1) {
                printf("序号导入\n");
                DWORD ordinal = (import_address_table->u1.Ordinal & 0b1111111111111111);
                printf("当前函数为序号导入， 序号为:%d\n", ordinal);
                printf("当序号导入， 函数地址是:%p\n", import_address_table->u1.Function);
            }
            else {
                printf("名称导入\n");
                IMAGE_IMPORT_BY_NAME* by_name = (IMAGE_IMPORT_BY_NAME*)(import_name_table->u1.AddressOfData + (BYTE*)dll);
                printf("当前按名称导入，函数名称是:%s\n", by_name->Name);
                printf("当前按名称导入， 函数地址是:%llx\n",import_address_table->u1.Function);//地址一定要从iat获取
                if (lstrcmpiA(by_name->Name, "strlen") == 0) {
                    printf("找到函数了");
                    using test = size_t(*)(const char*);
                    test t = (test)(import_address_table->u1.Function);
                    printf("ABCDEF的长度是:%u\n",t("abcdef"));
                    return 0;
                }
            }
            import_name_table++;
            import_address_table++;
        }

        my_import++;//跳到下一个dll结构
    }



}
