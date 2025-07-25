// 7.把exe复制进内存.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

//将exe写进内存， 之后再保存为另一个exe
/* 申请内存空间
LPVOID VirtualAlloc(
指定地址/NUL,
申请的内存大小,
MEM_COMMIT|MEM_RESERVE,
PAGE_EXECUTE_READWRITE
);
*/

int main()
{

    HANDLE old_f = CreateFileA("1.exe",GENERIC_ALL, 0,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!old_f) {
        printf("打开文件出错\n");
        printf("错误码为:%d\n", GetLastError());
        return 0;
    }
    DWORD old_f_size = GetFileSize(old_f, NULL);
    DWORD read_bytes{};
    BYTE* old_f_buff = new BYTE[old_f_size]{};
    if (!ReadFile(old_f, old_f_buff, old_f_size, &read_bytes, NULL)){
        printf("读取文件出错\n");
        printf("错误码为:%d\n", GetLastError());
        return 0;
    }
    printf("文件大小为: %d 字节\t读取了 %d 字节\n", old_f_size, read_bytes);


    //PE结构， 为了获取SizeOfImage
    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)old_f_buff;
    printf("DOS头标记:%x\n", dos_head->e_magic);
    IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)(old_f_buff+dos_head->e_lfanew);
    printf("PE头标记:%x\n", nt_head->Signature);
    //节头
    IMAGE_SECTION_HEADER* se_head = (IMAGE_SECTION_HEADER*)(
        old_f_buff+
        dos_head->e_lfanew+
        sizeof(nt_head->Signature)+
        sizeof(nt_head->FileHeader)+
        nt_head->FileHeader.SizeOfOptionalHeader
        );

    //准备写入内存
    DWORD size_of_mem = nt_head->OptionalHeader.SizeOfImage;//内存总大小
    ULONGLONG imagebase = nt_head->OptionalHeader.ImageBase;
    printf("imagebase为:%llx\n", imagebase);
    void* mem_address = VirtualAlloc((void*)imagebase, size_of_mem, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!mem_address) {
        printf("分配失败!\n");
        printf("分配失败错误码:%d\n", GetLastError());
        return 0;
    }
    //VirtualAlloc(NULL, size_of_mem, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);//随机帮我分配地址
    memset(mem_address, 0, size_of_mem);//内存清0
    printf("分配的内存地址为（mem_address）:%p\n", mem_address);

    //从文件复制内容到内存
    //复制头部所有数据
    memcpy_s(mem_address, nt_head->OptionalHeader.SizeOfHeaders, old_f_buff, nt_head->OptionalHeader.SizeOfHeaders);
    
    for (int i = 0; i < nt_head->FileHeader.NumberOfSections; i++) {
        BYTE swap_name[9]{};
        memcpy_s(swap_name, 9, se_head[i].Name, 8);
        printf("当前节的名字是:%s\n", swap_name);
        DWORD size_of_raw_data = se_head[i].SizeOfRawData;
        printf("当前节在文件中的大小:%x\n", size_of_raw_data);
        if (size_of_raw_data == 0) {
            printf("文件中的大小为0， 不用复制， 跳过\n");
            continue;
        }
        void* desc = (BYTE*)mem_address + se_head[i].VirtualAddress;//获取当前节在内存中的起始位置
        void* sour = old_f_buff + se_head[i].PointerToRawData;//获取当前节大文件中的起始位置
        //DWORD copy_size = se_head[i].SizeOfRawData > se_head[i].Misc.VirtualSize ? se_head[i].Misc.VirtualSize : se_head[i].SizeOfRawData;
        DWORD copy_size = min(se_head[i].SizeOfRawData,se_head[i].Misc.VirtualSize);
        if (se_head[i].SizeOfRawData >= se_head[i].Misc.VirtualSize) {
            memcpy_s(desc, copy_size, sour, copy_size);//只复制小的数据就行， 无论哪一边大， 都是0填充的， 不用复制
        }
        else {
            memcpy_s(desc, copy_size, sour, copy_size);//只复制小的数据就行， 无论哪一边大， 都是0填充的， 不用复制
            //大出来的要0填充
            DWORD padding = se_head[i].Misc.VirtualSize - se_head[i].SizeOfRawData;//填充的大小
            void* desc_padding = (BYTE*)mem_address + se_head[i].VirtualAddress + copy_size;//还有padding大小未填充
            memset(desc_padding, 0, padding);
        }
        printf("当前节已复制完成\n");

    }


    //nt_head->OptionalHeader.ImageBase+nt_head->OptionalHeader.AddressOfEntryPoint
    //前提是ImageBase与mem_address一样
    if ((ULONGLONG)mem_address == nt_head->OptionalHeader.ImageBase) {
        //IAT表未修复， 不能执行。
        // main执行前的CRT要用IAT， 没修复IAT都进入不到MAIN就异常出错了
        //当ImageBas == 加载址时， 重定位不用修复， 但IAT要修复
        //return 0;
        //void* EOP = (BYTE*)mem_address + nt_head->OptionalHeader.AddressOfEntryPoint;
        //((void(*)()) EOP) ();

        //当我指定非main函数生成exe时， 并不能用virtualloc申请指定的地址空间
    }
    else {
        printf("入口地址不一样，不能执行函数");
    }
    
    //文件打开EXE， 写进virtualloc申请的内存， 变成内存对齐状态
    //从内存中导出来， 写进一个新exe文件


    //mem_address  这个是申请的同存地址， 也就是PE文件最开始的位置
    //所有头部， 大小跟原始导入文件时是一样的。

    //创建一个文件
    HANDLE new_f = CreateFileA("new.exe", GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (new_f == INVALID_HANDLE_VALUE) {
        printf("创建新文件出错\n");
        printf("%d\n", GetLastError());
        return 0;
    }
    //复制内存中的所有头
    DWORD write_bytes{};
    WriteFile(new_f,mem_address,nt_head->OptionalHeader.SizeOfHeaders,&write_bytes,NULL);
    printf("写入了%d 字节\n", write_bytes);
    //写节
    for (int i = 0; i < nt_head->FileHeader.NumberOfSections;i++) {
        BYTE swap_name[9]{};
        memcpy_s(swap_name, 9, se_head[i].Name, 8);
        printf("---------------------------\n");
        printf("当前节的名字是:%s\n", swap_name);
        //复制节， 从内存复制到文件
        //节起始位置
        void* mem_se_start = (BYTE*)mem_address + se_head[i].VirtualAddress;
        printf("当前节在内存中的起始位置:%p\n", mem_se_start);
        //在内存中的在小
        DWORD mem_size = se_head[i].Misc.VirtualSize;
        //在文件中的大小
        DWORD in_file_size = se_head[i].SizeOfRawData;
        if (mem_size == 0) {
            printf("这个节大小为0, 跳过复制\n");
            continue;
        }
        if (mem_size >= in_file_size) {
            //文件指针要移动
            SetFilePointer(new_f, se_head[i].PointerToRawData,0,FILE_BEGIN);
            //内存读到位置偏移
            void* mem_post = (BYTE*)mem_address + se_head[i].VirtualAddress;
            DWORD get_mem_bytes{};
            WriteFile(new_f, mem_post, in_file_size, &get_mem_bytes, NULL);//文件这边小， 写文件这边的字节
            //内存中为了对齐， 在内存中可能不为0， 但在文件中，可以是0
            printf("写入了%d字节\n", get_mem_bytes);
        }
        else { //如果文件这边比内存那边的大， 多的位置要填充0
            //文件指针要移动
            SetFilePointer(new_f, se_head[i].PointerToRawData, 0, FILE_BEGIN);
            //内存读到位置偏移
            void* mem_post = (BYTE*)mem_address + se_head[i].VirtualAddress;
            DWORD get_mem_bytes{};
            WriteFile(new_f, mem_post, mem_size, &get_mem_bytes, NULL);//内存这边小， 写内存这边的字节大小
            printf("写入了%d字节\n", get_mem_bytes);
            DWORD padding = in_file_size - mem_size;//文件比内存那边大多少
            //大出来的位置要填充0
            //文件指针要移动
            SetFilePointer(new_f, se_head[i].PointerToRawData+mem_size, 0, FILE_BEGIN);
            BYTE* _zero = new BYTE[padding]{};//多少个0
            DWORD _zero_size_write{};
            WriteFile(new_f,_zero, padding,&_zero_size_write,NULL);//写入0
            printf("文件比内存中的大， 要写入0， 写入0的字节数为:%d\n", _zero_size_write);

        }

    }
    printf("从内存复制节到文件完成\n");

}

