// 9.导出表.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
int main()

{
	//我们现在用内存方式加载
	//如果你用createfile打开， 不能使用内存偏移

	HMODULE dll = LoadLibraryA("1.dll");
	if (!dll) {
		printf("导入dll出错\n");
		return 0;
	}
	

	//转换成PE结构
	IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)dll;
	IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)(dos_head->e_lfanew + (BYTE*)dll);
	printf("DOS标记:%x\n", dos_head->e_magic);
	printf("PE标记:%x\n", nt_head->Signature);
	//获取IMAGE_DATA_DIRECTOR结构
	IMAGE_DATA_DIRECTORY data = nt_head->OptionalHeader.DataDirectory[0];
	if (data.Size == 0 || data.VirtualAddress == 0) {
		printf("没有导出表\n");
		return 0;
	}
	//转换成导出表结构
	IMAGE_EXPORT_DIRECTORY* my_export = (IMAGE_EXPORT_DIRECTORY*)(data.VirtualAddress + (BYTE*)dll);

	BYTE* dll_name = my_export->Name + (BYTE*)dll;
	printf("当前的dll名字是:%s\n", dll_name);

	DWORD* name_array = (DWORD*)(my_export->AddressOfNames + (BYTE*)dll);//获取名称数组
	WORD* ordinals = (WORD*)(my_export->AddressOfNameOrdinals + (BYTE*)dll);//获取序号数组
	DWORD* functions = (DWORD*)(my_export->AddressOfFunctions + (BYTE*)dll);//获取函数数组
	//name -> 代理当前的dll名字 偏移
	//addressofName ->偏移， ['a'偏移, 'b'偏移, 'c'偏移]
	//addressordinal(偏移) -> [1,2,3,4]
	//addressoffunction偏移 -> [0x1偏移, 0x2偏移, 0x3偏移]
	for (int i = 0; i < my_export->NumberOfNames; i++) {
		printf("当前函数名字是:%s，当前下标:%d\n", name_array[i]+(BYTE*)dll,i);//数组每个元素+基地址
		WORD real_ord = ordinals[i];//获取序号
		//printf("序号为:%x\n", real_ord+my_export->Base);
		printf("序号为:%x\n", real_ord);
		//利用序号获取函数地址
		void* function_address = (void*)(functions[real_ord] + (BYTE*)dll);//利用序号获取函数地址，偏移+基地址
		printf("函数序号是: %d, 函数地址:%p\n", real_ord, function_address);
		((void(*)())  function_address) ();
		
	}

	
	//AddressOfNames -> 获取函数名， 当前函数名的下标， 放到AddressOfNameOrdinals[下标]后， 会获得函数序号
	//之后把这个序号名放到AddressOfFunctions[序号]后， 获得函数地址





	
}

