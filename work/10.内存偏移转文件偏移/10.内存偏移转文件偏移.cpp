// 10.内存偏移转文件偏移.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
DWORD mem_to_file(IMAGE_SECTION_HEADER*, IMAGE_NT_HEADERS*, DWORD);
int main()
{
	HANDLE f = CreateFileA("1.dll", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	DWORD file_size = GetFileSize(f, NULL);
	BYTE* file_buff = new BYTE[file_size];
	ReadFile(f, file_buff, file_size, NULL, NULL);
	//虽然你读进数组， 但没有成为真正的内存状态
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
	//我要获取导出表
	IMAGE_DATA_DIRECTORY data = nt_head->OptionalHeader.DataDirectory[0];

	if (data.Size == 0 && data.VirtualAddress == 0) {
		printf("没有导出表\n");
		return 1;
	}
	//获取真正的导出表结构
	//IMAGE_EXPORT_DIRECTORY* my_export = (IMAGE_EXPORT_DIRECTORY*) (200 + file_buff);
	
	//file_buff如果此地址为真正的内存地址， 而且你的1.dll是真正的内存装载， 可以这样加:
	//IMAGE_EXPORT_DIRECTORY* my_export = (IMAGE_EXPORT_DIRECTORY*) (data.VirtualAddress + file_buff);
	printf("IMAGE_DATA_DIRECTORY 中的内存地址为(RVA):%x\n", data.VirtualAddress);

	IMAGE_EXPORT_DIRECTORY* my_export = (IMAGE_EXPORT_DIRECTORY*)(mem_to_file(se_head, nt_head, data.VirtualAddress) + file_buff);
	printf("当前DLL带名字的导出函数个数是:%d\n", my_export->NumberOfNames);

	
	printf("当前dll的名字是:%s\n", mem_to_file(se_head,nt_head,my_export->Name)+file_buff);
	//打印导出表
	for (int i = 0; i < my_export->NumberOfNames;i++) {
		DWORD* address_name = (DWORD*)(mem_to_file(se_head, nt_head,my_export->AddressOfNames)+file_buff);//获取数组
		printf("%d 函数是:%s\n", i, (mem_to_file(se_head,nt_head,address_name[i])+file_buff));
		//获取函数序号数组
		WORD* ordinals_name = (WORD*)(mem_to_file(se_head, nt_head, my_export->AddressOfNameOrdinals) + file_buff); 
		DWORD ordinals = ordinals_name[i];
		//获取函数地址数组
		printf("%d 函数的序号为:%d\n",i, ordinals);
		
		DWORD* address_function = (DWORD*)(mem_to_file(se_head, nt_head, my_export->AddressOfFunctions) + file_buff);
		void* function = (mem_to_file(se_head,nt_head, address_function[ordinals]) + file_buff);
		printf("当前函数的地址为:%p\n", function);
		//((void(*)()) function) ();//调用函数， 还未装载入内存，不能调用


	}


	
}
//第一个参数是为了编历节， 第二个参数是为了获取节的大小，第三个数参为内存偏移
DWORD mem_to_file(IMAGE_SECTION_HEADER* se_head,IMAGE_NT_HEADERS * nt_head,DWORD address) {
	//打印节里的内存地址
	DWORD file_post{};//用于返回地址
	for (int i = 0; i < nt_head->FileHeader.NumberOfSections; i++) {
		printf("-------------------------------\n");
		BYTE name[9]{};
		memcpy_s(name, 9, se_head[i].Name, 8);
		printf("当前节的名字是:%s\n", name);
		DWORD vir_size = se_head[i].Misc.VirtualSize;
		printf("当前节在内存中的大小:%x\n", vir_size);
		printf("当前节在内存中的偏移地址:%x\n", se_head[i].VirtualAddress);
		printf("当前节所属内存范围:%x - %x \n", se_head[i].VirtualAddress, se_head[i].VirtualAddress + vir_size);
		printf("当前节所对应的文件偏移为:%x\n", se_head[i].PointerToRawData);
		DWORD se_virtualaddress_start = se_head[i].VirtualAddress;
		DWORD se_virtualaddress_end = se_head[i].VirtualAddress + vir_size;
		if ((se_virtualaddress_start <= address) && (address <= se_virtualaddress_end)) {
			printf("IMAGE_DATA_DIRECTORY 中的内存地址在这个节的内存地址范围里\n");
			DWORD virtual_post = address - se_virtualaddress_start;
			printf("IMAGE_DATA_DIRECTORY 中的内存地址离%x 的距离为:%x\n", se_virtualaddress_start, virtual_post);
			file_post = se_head[i].PointerToRawData + virtual_post;//转换成功的文件偏移
			printf("IMAGE_DATA_DIRECTORY 的文件偏移为:%x + %x =  %x\n", se_head[i].PointerToRawData, virtual_post, file_post);
			return file_post;
		}

	}
	//return 0;
}

