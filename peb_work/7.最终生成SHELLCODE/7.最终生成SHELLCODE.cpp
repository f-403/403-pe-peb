// 6.自定义ASCII字符串对比.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <Windows.h>
#include <winternl.h>

typedef struct _myLDR_DATA_TABLE_ENTRY
{
    struct _LIST_ENTRY InLoadOrderLinks;                                    //0x0
    struct _LIST_ENTRY InMemoryOrderLinks;                                  //0x10
    struct _LIST_ENTRY InInitializationOrderLinks;                          //0x20
    VOID* DllBase;                                                          //0x30
    VOID* EntryPoint;                                                       //0x38
    ULONG SizeOfImage;                                                      //0x40
    struct _UNICODE_STRING FullDllName;                                     //0x48
    struct _UNICODE_STRING BaseDllName;                                     //0x58
}myLDR_DATA_TABLE_ENTRY;

///*不分开写的话， 能定义在外面， 同时指定放到哪个节， 在内部不能指定放到哪个节

#pragma code_seg("ABCDE");

//在外面定义的话， 无论分开不分开， 都是存在.data段, 如果加了指定段， 都是存在指定段里
/*
//LoadLibraryA
__declspec(allocate("ABCDE")) char check_LoadLibraryA[] = "LoadLibraryA";
// GetProcAddress
__declspec(allocate("ABCDE"))  char check_GetProcAddress[] = "GetProcAddress";
//MessageBoxA
__declspec(allocate("ABCDE")) char check_MessageBoxA[] = "MessageBoxA";
//user32.dll
__declspec(allocate("ABCDE")) char check_user32dll[] = "USER32.dll";
*/

/*
__declspec(allocate("ABCDE")) char check_LoadLibraryA[] = { 'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0' };
// GetProcAddress
__declspec(allocate("ABCDE")) char check_GetProcAddress[] = { 'G', 'e', 't', 'P', 'r', 'o', 'c', 'A', 'd', 'd', 'r', 'e', 's', 's', '\0' };
// MessageBoxA
__declspec(allocate("ABCDE"))char check_MessageBoxA[] = { 'M', 'e', 's', 's', 'a', 'g', 'e', 'B', 'o', 'x', 'A', '\0' };
// user32.dll
__declspec(allocate("ABCDE"))char check_user32dll[] = { 'U', 'S', 'E', 'R', '3', '2', '.', 'd', 'l', 'l', '\0' };
*/



//在外面定义的话， 无论分开不分开， 都是存在.data段

/*
//在外面这样写， 会存在.data
char check_LoadLibraryA[] = { 'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0' };
// GetProcAddress
char check_GetProcAddress[] = { 'G', 'e', 't', 'P', 'r', 'o', 'c', 'A', 'd', 'd', 'r', 'e', 's', 's', '\0' };
// MessageBoxA
char check_MessageBoxA[] = { 'M', 'e', 's', 's', 'a', 'g', 'e', 'B', 'o', 'x', 'A', '\0' };
// user32.dll
char check_user32dll[] = { 'U', 'S', 'E', 'R', '3', '2', '.', 'd', 'l', 'l', '\0' };
*/

/*//在外面这样写存在.data段, 
char check_LoadLibraryA[] = "LoadLibraryA";
// GetProcAddress
char check_GetProcAddress[] = "GetProcAddress";
//MessageBoxA
char check_MessageBoxA[] = "MessageBoxA";
//user32.dll
char check_user32dll[] = "USER32.dll";
*/
int ascii_check(const char*, const char*);
void myshellcode()
{
    //在里面定义的话， 分开就存在.text段或自定义段， 合起来定义的话， 就存在.rdata段

    //分开定义， 会存在.text段或自定义段
    char gz[] = "GZ0668THISISATEST";
    // LoadLibraryA
    char check_LoadLibraryA[] = { 'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0' };
    // GetProcAddress
    char check_GetProcAddress[] = { 'G', 'e', 't', 'P', 'r', 'o', 'c', 'A', 'd', 'd', 'r', 'e', 's', 's', '\0' };
    // MessageBoxA
    char check_MessageBoxA[] = { 'M', 'e', 's', 's', 'a', 'g', 'e', 'B', 'o', 'x', 'A', '\0' };
    // user32.dll
    char check_user32dll[] = { 'U', 'S', 'E', 'R', '3', '2', '.', 'd', 'l', 'l', '\0' };
 
 /*
    ///合起来定义的话会存在.rdata节, 可以在PE中查到
    
    char check_LoadLibraryA[] = "LoadLibraryA";
    // GetProcAddress
    char check_GetProcAddress[] = "GetProcAddress";
    //MessageBoxA
    char check_MessageBoxA[] = "MessageBoxA";
    //user32.dll
    char check_user32dll[] = "USER32.dll";
*/    

    //user32.dll                               table_entry
    //PEB -> LDR -> LIST_ENTRY --InMemoryOrderModuleList-->
    PEB* peb = (PEB*)__readgsqword(0x60);
    //ldr
    PEB_LDR_DATA* ldr = peb->Ldr;
    //list_entry
    LIST_ENTRY* current_list = &ldr->InMemoryOrderModuleList;
    LIST_ENTRY* end_list = current_list->Blink;

    //自已定义的函数地址变量
    ULONGLONG  myGetProcAddress = 0;
    ULONGLONG myLoadLibraryA = 0;

    while (current_list != end_list) {

        myLDR_DATA_TABLE_ENTRY* table_entry = (myLDR_DATA_TABLE_ENTRY*)((char*)(end_list)-0x10);
        //std::wcout << L"导入的DLL为:" << table_entry->BaseDllName.Buffer << std::endl;
        //wprintf(L"导入的DLL名字为:%.*s\n", table_entry->BaseDllName.Length/sizeof(WCHAR), table_entry->BaseDllName.Buffer);
            //首先获取基地址， 也就是装载进内存后的地址
        //在导入的所有DLL中， 查找对应的函数

        void* base_address = table_entry->DllBase;
        //std::cout << "装载地址为:" << base_address << std::endl;
        //dos, nt
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base_address;
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(dos->e_lfanew + (char*)base_address);

        IMAGE_DATA_DIRECTORY data = nt->OptionalHeader.DataDirectory[0];
        if (data.Size == 0) {
            //std::cout << "没有导出表" << std::endl;
            end_list = end_list->Blink;
            continue;
        }
        //std::cout << "导出表大小为: " << data.Size << std::endl;

        IMAGE_EXPORT_DIRECTORY* my_export = (IMAGE_EXPORT_DIRECTORY*)((char*)base_address + data.VirtualAddress);

        for (int i = 0; i < my_export->NumberOfNames; i++) {

            //编历带名字的， 找到要找的函数， 比如LoadLabray....
            DWORD* name_table = (DWORD*)(my_export->AddressOfNames + (CHAR*)base_address);//名称表
            DWORD* address_table = (DWORD*)(my_export->AddressOfFunctions + (CHAR*)base_address);//地址表
            WORD* ordinals_table = (WORD*)(my_export->AddressOfNameOrdinals + (CHAR*)base_address);//序号表
            //获取名称
            char* function_name = name_table[i] + (char*)base_address;
            //std::cout << "函数名称是: " << function_name << std::endl;
            if (ascii_check(check_LoadLibraryA, function_name) == 1) {

                DWORD function_number = ordinals_table[i];
                //std::cout << "LoadLibraryA 函数对应的序号是:" << function_number << std::endl;
                myLoadLibraryA = (ULONGLONG)(address_table[function_number] + (char*)base_address);
                //std::cout << "LoadLibraryA 函数对应的地址是:" << std::hex << myLoadLibraryA << std::endl;
                //using LOAD = HMODULE (WINAPI*)(LPCSTR );
                //LOAD load = (LOAD)function_address;
                //HMODULE u32dll = load("user32.dll");

                //using BOX = int(*)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
                //BOX box = (BOX)GetProcAddress(u32dll, "MessageBoxA");//在写SHELLCODE时， GetProcAddress也要用同样方法查找， 不能直接调用
                //你要知道它在哪个DLL，
                //box(0,0,0,0);
            }
            //GetProcAddress
            if (ascii_check(check_GetProcAddress, function_name) == 1) {

                DWORD function_number = ordinals_table[i];
                //std::cout << "GetProcAddress 函数对应的序号是:" << function_number << std::endl;
                myGetProcAddress = (ULONGLONG)(address_table[function_number] + (char*)base_address);
                //std::cout << "GetProcAddress 函数对应的地址是:" << std::hex << myGetProcAddress << std::endl;

            }

        }

        end_list = end_list->Blink;

    }
    //std::cout << std::hex << myGetProcAddress << std::endl;
    //std::cout << std::hex << myLoadLibraryA << std::endl;
    if (myGetProcAddress != 0 && myLoadLibraryA != 0) {
        //std::cout << "找到两个地址" << std::endl;
        //std::cout << std::hex << myGetProcAddress << std::endl;
        //std::cout << std::hex << myLoadLibraryA << std::endl;
        //测试
        using LOAD = HMODULE(WINAPI*)(LPCSTR);
        LOAD load = (LOAD)myLoadLibraryA;
        HMODULE u32dll = load(check_user32dll);

        //
        using GET = FARPROC(*)(HMODULE, LPCSTR);
        GET get = (GET)myGetProcAddress;
        using BOX = int(*)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);//MessageBoxA

        BOX box = (BOX)get(u32dll, check_MessageBoxA);//在写SHELLCODE时， GetProcAddress也要用同样方法查找， 不能直接调用
        //你要知道它在哪个DLL，
        box(0, 0, 0, 0);
    }


}


int ascii_check(const char* sor, const char* desc) {
    //如果是一样的字符串， 返回1
    int sor_size = 0;
    while (sor[sor_size]) {
        sor_size++;
    }
    //std::cout << "原始字符串长度:" << sor_size << std::endl;

    int desc_size = 0;
    while (desc[desc_size]) {
        desc_size++;
    }
    //std::cout << "对比的字符串长度:" << desc_size << std::endl;

    if (sor_size != desc_size) return 0;
    int check = 0;
    for (; check < sor_size; check++) {
        if (sor[check] == desc[check]) continue;
        break;
    }
    if (sor_size != check) return 0;
    return 1;
}

