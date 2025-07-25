// 3.使用自带的结构体打印dos_head与nt头标记.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
int main()
{
    HANDLE f = CreateFileA("1.exe", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //转成dos_header

    DWORD read_bytes{};//读取的字节数
    DWORD file_size = GetFileSize(f, NULL);
    //BYTE file_buff[file_size];
    printf("文件大小为: %d\n", file_size);
    BYTE* file_buff = new BYTE[file_size];
    if (!ReadFile(f, file_buff, file_size, &read_bytes, NULL)) {
        printf("读取PE文件出错");
        return 0;
    }
    printf("读取到的文件大小为: %d\n", read_bytes);
    printf("file_buff地址为:%p\n", file_buff);

    //找DOS_HEADER
    IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)file_buff;
    //打印DOS头标记与e_lfanew的值
    printf("dos头标记为:%x\n", dos_head->e_magic);
    printf("dos头中的e_lfanew值为:%x\n", dos_head->e_lfanew);

    if (dos_head->e_magic == IMAGE_DOS_SIGNATURE) {
        printf("这是一个dos头\n");
    }

    delete[] file_buff;

}

