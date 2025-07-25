// 4.nt中的file_head与op_head.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
int main()
{
    HANDLE f = CreateFileA("d.exe", GENERIC_ALL,0,NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if (f == INVALID_HANDLE_VALUE) {
        printf("打开文件失败\n");
        return 0;
    }
    DWORD file_size = GetFileSize(f, NULL);
    BYTE* file_buff = new BYTE[file_size];
    //读取进file_buff
    DWORD read_bytes{};
    if (!ReadFile(f, file_buff, file_size, &read_bytes, NULL)) {
        printf("读取PE文件失败\n");
        return 0;
    }
    printf("文件大小为: %d 字节\t读取了: %d 字节\n", file_size, read_bytes);

    //dos头
    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)file_buff;
    //printf("dos头标记:%x\n", *(WORD*)dos_head);
    printf("dos头标记:%x\n", dos_head->e_magic);
    printf("e_lfanew的值为:%x\n", dos_head->e_lfanew);

    //获取nt头
    IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)(file_buff + dos_head->e_lfanew);//nt头最开始的位置
    //printf("nt头标记:%x\n", *(DWORD*)nt_head);
    printf("nt头标记:%x\n", nt_head->Signature);
    printf("nt头标记（字符）:%s%\n", (BYTE*)(&nt_head->Signature));
    //nt_file_head
    //printf("Machine:%x\n", *(WORD*)((BYTE*)nt_head+4));
    printf("Machine:%x\n", nt_head->FileHeader.Machine);
    printf("Characteristics:%x\n", nt_head->FileHeader.Characteristics);
    printf("节的个数: %d\n", nt_head->FileHeader.NumberOfSections);
    //可选头
    printf("ImageBase(打算载入的地址)：%llx\n", nt_head->OptionalHeader.ImageBase);//ulonglong, 要加ll， 要不会截断
    printf("程序执行入口(EOP):%x\n", nt_head->OptionalHeader.AddressOfEntryPoint);
    printf("载入内存后的大小为:%x\n", nt_head->OptionalHeader.SizeOfImage);



    delete[]file_buff;


}

