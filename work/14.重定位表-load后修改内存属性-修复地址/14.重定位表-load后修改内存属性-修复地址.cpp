// 13.重定位表.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<Windows.h>

int main()
{

	HMODULE f_buff = LoadLibraryA("1.exe");

	IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)((BYTE*)f_buff);
	IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)((BYTE*)f_buff + dos_head->e_lfanew);
	printf("DOS头:%x\n", dos_head->e_magic);
	printf("PE头:%x\n", nt_head->Signature);
	//获取重定位表
	IMAGE_DATA_DIRECTORY my_data = (IMAGE_DATA_DIRECTORY)nt_head->OptionalHeader.DataDirectory[5];
	//获重位表
	IMAGE_BASE_RELOCATION* base_rel = (IMAGE_BASE_RELOCATION*)(my_data.VirtualAddress + (BYTE*)f_buff);

	//修改内存为可读写，使得后面修改地址时能正确修改内存里的址
	DWORD oldprotect{};
	VirtualProtect(f_buff,nt_head->OptionalHeader.SizeOfImage,PAGE_READWRITE,&oldprotect);

	if (base_rel->SizeOfBlock == 0) {
		printf("没有重定位表\n");
		return 0;
	}
	DWORD fix_post = (ULONGLONG)f_buff - nt_head->OptionalHeader.ImageBase;
	printf("修复的差值为:%x\n", fix_post); //return 0;

	while (base_rel->SizeOfBlock != 0) {
		printf("当前重定位表的总大小为:%x\n", base_rel->SizeOfBlock);
		//获取项的个数， 也就是那个WORD数组的个数
		DWORD item = (base_rel->SizeOfBlock - 8) / 2;
		printf("项的个数为:%d\n", item);
		printf("当前重定位表的偏移地址为:%x, 这地址的目的是为了获取要修复的地址\n", base_rel->VirtualAddress);
		//DWORD* pFixAddr = (DWORD*)(base + VirtualAddress + offset);

		//打印项里的WORD
		WORD* typeoff = (WORD*)((BYTE*)base_rel + 8);
		for (int i = 0; i < item;i++) {
			DWORD type = (*typeoff >> 12);
			printf("高4位:%x\n", type);
			DWORD offset = (*typeoff) & 0xfff;//& 0b1111 1111 1111 
			printf("低12位:%x\n", offset);

			//找到地址
			//判断是32/64
			if (type == 0x3) {//32
				printf("修复32位地址\n");
				printf("修复64位地址\n");
				ULONGLONG* fix_addr = (ULONGLONG*)((BYTE*)f_buff + base_rel->VirtualAddress + offset);//找到地址
				//真正修复
				//[0x1234]
				*fix_addr = *fix_addr + fix_post;//修复地址
			}
			if (type == 0xa) {//64
				printf("修复64位地址\n");
				ULONGLONG *fix_addr = (ULONGLONG*)((BYTE*)f_buff + base_rel->VirtualAddress + offset);//找到地址
				//真正修复
				//[0x1234]
				*fix_addr = *fix_addr + fix_post;//修复地址
				//内存保护
				//VirtualProtect(f_buff, nt_head->OptionalHeader.SizeOfImage, PAGE_READWRITE, &oldprotect);//用于能够修改内存
			}

			typeoff++;
			
		}


		base_rel = (IMAGE_BASE_RELOCATION*)((BYTE*)base_rel + base_rel->SizeOfBlock);//跳转到下一个结构
	}

	VirtualProtect(f_buff, nt_head->OptionalHeader.SizeOfImage, oldprotect, &oldprotect);

	printf("实际装载地址:%llx\n", f_buff);
	printf("ImageBse:%llx\n", nt_head->OptionalHeader.ImageBase);


}

