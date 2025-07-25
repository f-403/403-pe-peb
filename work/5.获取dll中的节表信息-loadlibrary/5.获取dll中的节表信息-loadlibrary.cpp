#include <Windows.h>
#include <iostream>

int main() {

	HMODULE dll = LoadLibraryA("1.dll");//ImageBase
	//option_head -> imagebase 里面的值与这个loadlibrary返回值相等
	//loadlibrary会自动帮你内存对齐， 一切帮你安排好了的
	if (!dll) {
		printf("导入dll出错");
		return 0;
	}
	//获取PE结构
	IMAGE_DOS_HEADER* dos_head = (IMAGE_DOS_HEADER*)dll;
	printf("DOS头标记:%x\n", dos_head->e_magic);
	IMAGE_NT_HEADERS* nt_head = (IMAGE_NT_HEADERS*)(dos_head->e_lfanew + (BYTE*)dll);
	printf("NT头标记:%x\n", nt_head->Signature);
	printf("NT可选头中的ImageBase地址:%llx\n", nt_head->OptionalHeader.ImageBase);
	printf("LoadLibrary载入地址:%llx\n", (ULONGLONG)dll);
	//如果ImageBase与载入地址不一样， 就要修复IAT表， 重定位表

	//计算节表头偏移并转换成节表数据结构
	IMAGE_SECTION_HEADER* se_head = (IMAGE_SECTION_HEADER*)(
		(BYTE*)dll+
		dos_head->e_lfanew+
		sizeof(nt_head->Signature)+
		sizeof(nt_head->FileHeader)+
		nt_head->FileHeader.SizeOfOptionalHeader
		);

	printf("节的数量:%d\n", nt_head->FileHeader.NumberOfSections);

	for (int i = 0; i < nt_head->FileHeader.NumberOfSections; i++) {
		BYTE swap_name[9]{};
		memcpy_s(swap_name, 9, se_head[i].Name, 8);
		printf("%d - 节的名称:%s\n", i, swap_name);
		printf("当前节在内存中的大小为:%x\n", se_head[i].Misc.VirtualSize);//在内存中的大小
	}
	FreeLibrary(dll);

	//文件方式打开
	HANDLE f = CreateFileA("1.dll", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == INVALID_HANDLE_VALUE) {
		printf("打开dll文件失败");
		return 0;
	}
	//转化成pe结构操作
	DWORD file_size = GetFileSize(f, NULL);
	BYTE* file_buff = new BYTE[file_size]{};
	DWORD read_bytes{};
	if (!ReadFile(f, file_buff, file_size, &read_bytes, NULL)) {
		printf("读取失败\n");
		return 0;
	}
	printf("文件数组地址:%p\n", file_buff);
	printf("文件大小: %d 字节\t读进数组的字节数:%d\n", file_size, read_bytes);
	//首先转换为dos头
	IMAGE_DOS_HEADER* f_dos_head = (IMAGE_DOS_HEADER*)file_buff;
	printf("文件打开后的DOS头标记:%x\n", f_dos_head->e_magic);
	//转换为NT头
	IMAGE_NT_HEADERS* f_nt_head = (IMAGE_NT_HEADERS*)(f_dos_head->e_lfanew + file_buff);
	printf("文件打开后的NT头标记:%x\n", f_nt_head->Signature);
	printf("文件打开，并读进数组后的数组地址(相当于ImageBase):%p\n", file_buff);
	printf("nt可选头中的建议加载地址(期望加载的ImageBase):%p\n", (void*)f_nt_head->OptionalHeader.ImageBase);
	//获取节头
	IMAGE_SECTION_HEADER* f_se_head = (IMAGE_SECTION_HEADER*)(
		file_buff+
		f_dos_head->e_lfanew+
		sizeof(f_nt_head->Signature)+
		sizeof(f_nt_head->FileHeader)+
		f_nt_head->FileHeader.SizeOfOptionalHeader
		);
	printf("节的数量:%d\n", f_nt_head->FileHeader.NumberOfSections);

	for (int i = 0; i < f_nt_head->FileHeader.NumberOfSections; i++) {
		BYTE swap_name[9]{};
		memcpy_s(swap_name, 9, f_se_head[i].Name, 8);
		printf("%d - 节的名称:%s\n", i, swap_name);
		printf("当前节用文件方式打开后的大小:%x\n", f_se_head[i].SizeOfRawData);//文件中的实际大小
	}

	//内存真正载入时获取大小:se_head[i].Misc.VirtualSize
	//文件打开时获取大小:f_se_head[i].SizeOfRawData


}