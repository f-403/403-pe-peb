// 15.修复重定位表与iat表-执行EOP.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
int main()
{
    HANDLE file = CreateFileA("1.exe", GENERIC_ALL, 0, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (file == INVALID_HANDLE_VALUE) {
        printf("文件打开失败\n");
        return 1;
    }
    DWORD file_size = GetFileSize(file, NULL);
    BYTE* file_buff = new BYTE[file_size];

    DWORD read_bytes{};
    ReadFile(file, file_buff, file_size, &read_bytes, NULL);
    printf("文件大小为:%d 字节, 读入了 %d 字节\n", file_size, read_bytes);

    //PE结构
    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)file_buff;
    IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)(file_buff + dos_head->e_lfanew);
    IMAGE_SECTION_HEADER* se_head = (IMAGE_SECTION_HEADER*)(
        file_buff+
        dos_head->e_lfanew+
        sizeof(nt_head->Signature)+
        sizeof(nt_head->FileHeader)+
        nt_head->FileHeader.SizeOfOptionalHeader
        );
    printf("DOS标记:%x\n", dos_head->e_magic);
    printf("PE标记:%x\n", nt_head->Signature);
    //申请内存
    //void* mem_address = VirtualAlloc((VOID*)nt_head->OptionalHeader.ImageBase, nt_head->OptionalHeader.SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    //if (!mem_address) {
    //    printf("分配指定地址失败\n");
    //    return 1;
    //}
    void* mem_address = VirtualAlloc(NULL, nt_head->OptionalHeader.SizeOfImage, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!mem_address) {
        printf("分配指定地址失败\n");
        return 1;
    }
    memset(mem_address, 0, nt_head->OptionalHeader.SizeOfImage);//内存清0
    //自动分配地址

    //复制头部
    memcpy_s(mem_address, nt_head->OptionalHeader.SizeOfHeaders, file_buff, nt_head->OptionalHeader.SizeOfHeaders);//
    
    //复制节
    for (int i = 0; i < nt_head->FileHeader.NumberOfSections; i++) {
        BYTE name[9]{};
        memcpy_s(name, 9, se_head[i].Name, 8);
        printf("当前节是:%s\n", name);
        //内容是从文件复制到内存，大小为文件与内存大小中的小值
        //内存块比文件块大的话， 要填0
        DWORD copy_size = min(se_head[i].Misc.VirtualSize, se_head[i].SizeOfRawData);//取最小的
        if (se_head[i].Misc.VirtualSize == 0) continue;//是0的话， 跳过这个节， 不用复制
        memcpy_s((BYTE*)mem_address + se_head[i].VirtualAddress, copy_size, file_buff + se_head[i].PointerToRawData, copy_size);
        printf("当前节已复制完成\n");
    }
    //
    void* EOP = nt_head->OptionalHeader.AddressOfEntryPoint + (BYTE*)mem_address;//eop

    //((void(*)()) EOP) ();
    //下面修复重定位与IAT

    //重定位修复
    IMAGE_DATA_DIRECTORY re_data = nt_head->OptionalHeader.DataDirectory[5];
    
    //计算装载地址与imagebase的差值，用于修复重定位表里的地址
    LONGLONG delta = (ULONGLONG)mem_address - nt_head->OptionalHeader.ImageBase;
    if (re_data.Size != 0) {
        printf("存在重定位表，准备修复\n");
        IMAGE_BASE_RELOCATION* reloc = (IMAGE_BASE_RELOCATION*)(re_data.VirtualAddress + (BYTE*)mem_address);
        while (reloc->SizeOfBlock != 0) {
            DWORD item = (reloc->SizeOfBlock - 8) / 2;//一共有多少个项
            WORD* typeoffset = (WORD*)((BYTE*)reloc + 8);//获取每项的起始位置
            for (int i = 0; i < item; i++) {
                //在这里准备修复
                WORD type = (*typeoffset) >> 12;//高4位
                WORD offset = (*typeoffset) & 0xfff;//低12位
                if (type == 0xa) {
                    printf("64位修复\n");
                    ULONGLONG* fix_addr = (ULONGLONG*)((BYTE*)mem_address + reloc->VirtualAddress + offset);//64地址
                    //先找到原本要修复的地址   
                    *fix_addr = *fix_addr + delta;//把指针指向的值改变
                }
                if (type == 0x3) {
                    DWORD* fix_addr = (DWORD*)((BYTE*)mem_address + reloc->VirtualAddress + offset);//32地址
                    //先找到原本要修复的地址
                    *fix_addr = *fix_addr + (DWORD)delta;//把指针指向的值改变
                    printf("32位修复\n");
                }
                typeoffset++;//下一个typeoffset项
            }
            reloc = (IMAGE_BASE_RELOCATION*)((BYTE*)reloc + reloc->SizeOfBlock);//下一个重定位结构
        }

    }
    else {
        printf("没有重定位表，不用修复\n");
    }
    printf("重定位表修复成功!\n");

    //IAT表修复

    IMAGE_DATA_DIRECTORY im_data = nt_head->OptionalHeader.DataDirectory[1];//导入表
    if (im_data.VirtualAddress != 0) {
        printf("导入表要修复\n");
        IMAGE_IMPORT_DESCRIPTOR* my_import = (IMAGE_IMPORT_DESCRIPTOR*)(im_data.VirtualAddress + (BYTE*)mem_address);
        for (;;) {
            if (my_import->FirstThunk == 0) break;
            BYTE* dll_name = my_import->Name + (BYTE*)mem_address;
            IMAGE_THUNK_DATA* name_table = (IMAGE_THUNK_DATA*)((BYTE*)mem_address + my_import->OriginalFirstThunk);//名称表
            IMAGE_THUNK_DATA* address_table = (IMAGE_THUNK_DATA*)((BYTE*)mem_address + my_import->FirstThunk);//地址表
            printf("当前导入的dll名字是:%s\n", dll_name);

            for (;;) {
                if (name_table->u1.AddressOfData == 0) break;
                //判断是序号还是名称导入
                if ((name_table->u1.Ordinal >> 63) & 1) {
                    printf("序号导入\n");
                    DWORD function_number = name_table->u1.Ordinal & 0xffff;//提取低16
                    printf("函数序号为:%d\n", function_number);
                    HMODULE load_dll_check = LoadLibraryA((LPCSTR)dll_name);
                    if (!load_dll_check) {
                        printf("导入dll->%s失败\n", dll_name);
                        return 1;
                    }
                    void* swap_function_address = GetProcAddress(load_dll_check, (LPCSTR)function_number);//获取函数地址
                    address_table->u1.Function = (ULONGLONG)swap_function_address;//修复函数地址
                    printf("函数地址修复成功\n");

                }
                else {
                    printf("名称导入\n");
                    IMAGE_IMPORT_BY_NAME* function_name = (IMAGE_IMPORT_BY_NAME*)(name_table->u1.AddressOfData + (BYTE*)mem_address);
                    printf("函数名为:%s\n", function_name->Name);
                    HMODULE load_dll_check = LoadLibraryA((LPCSTR)dll_name);
                    if (!load_dll_check) {
                        printf("导入dll->%s失败\n", dll_name);
                        return 1;
                    }
                    void* swap_function_address = GetProcAddress(load_dll_check, function_name->Name);//获取函数地址
                    address_table->u1.Function = (ULONGLONG)swap_function_address;//修复函数地址
                    printf("函数地址修复成功\n");

                }
                name_table++;
                address_table++;
            }
            
            my_import++;
        }

    }
    else {
        printf("不存在导入表\n");

    }

    ((void(*)()) EOP) ();

    delete[] file_buff;
}

