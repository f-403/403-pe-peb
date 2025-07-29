// 3.提取文件中的16进制-并去除CC.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

int main()
{
    HANDLE f = CreateFileA(".text.txt", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD f_size = GetFileSize(f, NULL);
    BYTE *f_buff = new BYTE[f_size]{};
    ReadFile(f, f_buff, f_size, NULL, NULL);
    HANDLE new_f = CreateFileA("shellcode.txt", GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    for (int i = 0; i < f_size; i++) {
        if (f_buff[i] != 0x111cc) {
            if (i % 16 == 0) printf("\n");
            printf("0x%02x,", f_buff[i]);
            WriteFile(new_f, &f_buff[i], 1, NULL, NULL);
        }
    }
    CloseHandle(f);
    CloseHandle(new_f);
    delete[] f_buff;
}

