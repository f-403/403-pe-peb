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

    //DWORD* e_lfanew为nt头起始偏移量
    //dos_header大小为0x40字节(64字节)
    //dos_stub大小为: pe头偏移量 - dos头大小(0x40)
    //dos_sub起始位置: file_buff + 0x40
    unsigned char* dos_stub = file_buff + 0x40;//dos_stub起始位置
    DWORD dos_stub_size = *e_lfanew - 0x40;//dos_stub总大小
    for (int i = 0; i < dos_stub_size; i++) {
        printf("%02x ",dos_stub[i]);
    }
    for (int i = 0; i < dos_stub_size; i++) {
        printf("%c", dos_stub[i]);
    }

    return 0;
}