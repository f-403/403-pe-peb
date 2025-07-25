// 13.重定位表.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<Windows.h>

int main()
{

	HMODULE f_buff = LoadLibraryA("1.exe");

	IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)((BYTE*)f_buff);
	IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)((BYTE*)f_buff +dos_head->e_lfanew);
	printf("DOS头:%x\n", dos_head->e_magic);
	printf("PE头:%x\n", nt_head->Signature);
	//获取重定位表
	IMAGE_DATA_DIRECTORY my_data = (IMAGE_DATA_DIRECTORY)nt_head->OptionalHeader.DataDirectory[5];
	//获重位表
	IMAGE_BASE_RELOCATION* base_rel = (IMAGE_BASE_RELOCATION*)(my_data.VirtualAddress + (BYTE*)f_buff);

	if (base_rel->SizeOfBlock == 0) {
		printf("没有重定位表\n");
		return 0;
	}
	while (base_rel->SizeOfBlock != 0) {
		printf("当前重定位表的总大小为:%x\n", base_rel->SizeOfBlock);
		//获取项的个数， 也就是那个WORD数组的个数
		DWORD item = (base_rel->SizeOfBlock - 8) / 2;
		printf("项的个数为:%d\n", item);
		printf("当前重定位表的偏移地址为:%x, 这地址的目的是为了获取要修复的地址\n", base_rel->VirtualAddress);
		//DWORD* pFixAddr = (DWORD*)(base + VirtualAddress + offset);
		  base_rel = (IMAGE_BASE_RELOCATION*)((BYTE*)base_rel + base_rel->SizeOfBlock);//跳转到下一个结构
	}

	printf("实际装载地址:%llx\n", f_buff);
	printf("ImageBse:%llx\n", nt_head->OptionalHeader.ImageBase);

	
}

