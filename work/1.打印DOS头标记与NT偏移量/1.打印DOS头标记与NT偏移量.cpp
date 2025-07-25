#include <windows.h>
#include <iostream>

int main() {
    HANDLE f = CreateFileA("1.exe", GENERIC_ALL, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE) {
        printf("打开文件出错");
        return 0;
    }

    unsigned char file_buff[1024]{};
    DWORD read_bytes{};
    if (!ReadFile(f, file_buff, 1024, &read_bytes, NULL)) {
        printf("读取文件出错\n");
        return 0;
    }
    printf("读取了 %d 字节\n", read_bytes);
    printf("数组首地址为:%p\n", file_buff);
    printf("DOS头标记:%c%c\n", file_buff[0], file_buff[1]); //MZ
    //e_lfanew为偏移0x3c位置， 大小为DWORD
    DWORD* e_lfanew = (DWORD*)(file_buff + 0x3c);
    printf("e_lfanew的值为:%x\n", *e_lfanew);

    unsigned char* nt_head = file_buff + (*e_lfanew); //nt头的标记
    printf("nt头标记:%x\n", *((DWORD*)nt_head));
    printf("nt头字符串: %s\n", nt_head);//PE\O\O




    return 0;
}