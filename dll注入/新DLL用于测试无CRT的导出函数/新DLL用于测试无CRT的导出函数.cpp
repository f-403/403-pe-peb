// 一个地址无关的函数-用于反射加载.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <string>
//用于PEB结构
typedef struct _myPEB64
{
    UCHAR InheritedAddressSpace;                                            //0x0
    UCHAR ReadImageFileExecOptions;                                         //0x1
    UCHAR BeingDebugged;                                                    //0x2
    union
    {
        UCHAR BitField;                                                     //0x3
        struct
        {
            UCHAR ImageUsesLargePages : 1;                                    //0x3
            UCHAR IsProtectedProcess : 1;                                     //0x3
            UCHAR IsImageDynamicallyRelocated : 1;                            //0x3
            UCHAR SkipPatchingUser32Forwarders : 1;                           //0x3
            UCHAR IsPackagedProcess : 1;                                      //0x3
            UCHAR IsAppContainer : 1;                                         //0x3
            UCHAR IsProtectedProcessLight : 1;                                //0x3
            UCHAR IsLongPathAwareProcess : 1;                                 //0x3
        };
    };
    UCHAR Padding0[4];                                                      //0x4
    ULONGLONG Mutant;                                                       //0x8
    ULONGLONG ImageBaseAddress;                                             //0x10
    ULONGLONG Ldr;                                                          //0x18
    ULONGLONG ProcessParameters;                                            //0x20

}myPEB, * myPPEB;



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
}myLDR_DATA_TABLE_ENTRY, myPLDR_DATA_TABLE_ENTRY;
//用于修复PE文件并返回修复后的地址
extern "C" __declspec(dllexport) DWORD WINAPI check_dll(void*);//用于反射加载自身
__forceinline int my_ascii_cmp(const char* sour, const  char* desc);//用于对比字符串
__forceinline ULONGLONG* my_check_function_address(myLDR_DATA_TABLE_ENTRY* table_entry, ULONGLONG dll_base, char* check_function_name);//用于查找特定函数地址
ULONGLONG mem_to_file(IMAGE_SECTION_HEADER* se_head, IMAGE_NT_HEADERS* nt_head, ULONGLONG address);
//函数原型
typedef HMODULE(*myLoadLibraryA_source)(LPCSTR);
typedef FARPROC(*myGetProcAddress_source)(HMODULE, LPCSTR);
typedef LPVOID(*myVirtualAlloc_source)(LPVOID, SIZE_T, DWORD, DWORD);
typedef void* (*mymemset_source)(void*, int, size_t);
typedef errno_t(*mymemcpy_s_source)(void* const, rsize_t const, void const* const, rsize_t const);
typedef DWORD(*myGetCurrentProcessId_soruce)(VOID);//未使用


void hello();

void hello() {
    MessageBoxA(NULL, std::to_string(GetCurrentProcessId()).c_str(), "GetCurrentProcessId:", 0);
}
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // DLL 模块的句柄（基址）
    DWORD fdwReason,     // 调用原因（进程/线程附加/分离）
    LPVOID lpvReserved   // 保留参数，通常不用
)

{

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // 进程加载 DLL，执行初始化代码
        hello();
        break;
    case DLL_THREAD_ATTACH:
        // 新线程创建

        break;
    case DLL_THREAD_DETACH:
        // 线程退出
        break;
    case DLL_PROCESS_DETACH:
        // 进程卸载 DLL，执行清理代码
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) DWORD WINAPI check_dll(void* dll_address_) {

    uintptr_t dll_address = (uintptr_t)dll_address_;
    //查找PEB， 找到三个函数地址, 
    //GetProcAddress;
    //VirtualAlloc;
    //LoadLibraryA;
    char myLoadLibraryA[] = {
    'L', 'o', 'a', 'd', 'L', 'i', 'b', 'r', 'a', 'r', 'y', 'A', '\0'
    };

    char myGetProcAddress[] = {
        'G', 'e', 't', 'P', 'r', 'o', 'c', 'A', 'd', 'd', 'r', 'e', 's', 's', '\0'
    };

    char myVirtualAlloc[] = {
        'V','i','r','t','u','a','l','A','l','l','o','c','\0'
    };
    char mymemset[] = { 'm','e','m','s','e','t','\0' };
    char mymemcpy_s[] = { 'm','e','m','c','p','y','_','s','\0' };
    char myGetCurrentProcessId[] = { 'G','e','t','C','u','r','r','e','n','t','P','r','o','c','e','s','s','I','d','\0' };


    myPEB* mypeb = (myPEB*)__readgsqword(0x60);
    PEB_LDR_DATA* ldr = (PEB_LDR_DATA*)mypeb->Ldr;//ldr结构
    LIST_ENTRY* current_list = &ldr->InMemoryOrderModuleList;
    LIST_ENTRY* end_list = current_list->Blink;//获取链表头准备编历
    ULONGLONG* myLoadLibraryA_addr = 0;
    ULONGLONG* myGetProcAddress_addr = 0;
    ULONGLONG* myVirtualAlloc_addr = 0;
    ULONGLONG* mymemset_addr = 0;
    ULONGLONG* mymemcpy_s_addr = 0;
    ULONGLONG* myGetCurrentProcessId_addr = 0;
    //用来存放真正的地址
    myGetCurrentProcessId_soruce m_g_p_id = NULL;
    mymemcpy_s_source m_cpy_s = NULL;
    mymemset_source m_set = NULL;
    myGetProcAddress_source my_g_p_a = NULL;
    myLoadLibraryA_source my_l_l = NULL;
    myVirtualAlloc_source my_v_a = NULL;


    while (current_list != end_list) {
        int offset = (int)&((myLDR_DATA_TABLE_ENTRY*)0)->InMemoryOrderLinks;//偏移量
        //printf("偏移量为:%x\n", offset);
        myLDR_DATA_TABLE_ENTRY* table_entry = (myLDR_DATA_TABLE_ENTRY*)((char*)end_list - offset);
        //wprintf(L"%.*ls\n", (int)table_entry->BaseDllName.Length / sizeof(WCHAR), table_entry->BaseDllName.Buffer);

        //如果找到， 就不再查找
        if (!myLoadLibraryA_addr) {
            myLoadLibraryA_addr = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, myLoadLibraryA);//返回找到的函数地址
            //if (get_load_dll) return 0;
        }
        if (!myGetProcAddress_addr) {
            myGetProcAddress_addr = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, myGetProcAddress);//返回找到的函数地址
            //if (get_addr_address) return 0;
        }

        if (!myVirtualAlloc_addr) {
            myVirtualAlloc_addr = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, myVirtualAlloc);//返回找到的函数地址
            //if (get_addr_address) return 0;
        }
        if (!mymemset_addr) {
            mymemset_addr = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, mymemset);//返回找到的函数地址
            //if (get_addr_address) return 0;
        }
        if (!mymemcpy_s_addr) {
            mymemcpy_s_addr = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, mymemcpy_s);//返回找到的函数地址
            //if (get_addr_address) return 0;
        }
        if (!myGetCurrentProcessId_addr) {
            myGetCurrentProcessId_addr = my_check_function_address(table_entry, (ULONGLONG)table_entry->DllBase, myGetCurrentProcessId);//返回找到的函数地址
            //if (get_addr_address) return 0;
        }


        //如果提前找到， 退出
        if (myLoadLibraryA_addr && myGetProcAddress_addr && myVirtualAlloc_addr && mymemset_addr && mymemcpy_s_addr && myGetCurrentProcessId_addr) {
            //std::cout << "找到了loadlibrarya/getprocaddress/virtualalloc 3个地址" << std::endl; return 0;
            break;
        }

        end_list = end_list->Blink;//跳到下一个链表
    }
    //int check = my_ascii_cmp("abcdef12f3456", "abcdef12f3456");
    //printf("check:%d\n", check);
    //printf("%p\t%p\t%p\n", get_load_dll, get_addr_address, get_messaageboxa_addr);
    //上面打印，可以看到没有获取到MessageBoxA的地址
    //如要找到了地址， 就load user32.dll做测试
    //以下为测试
    if (myLoadLibraryA_addr && myGetProcAddress_addr && myVirtualAlloc_addr) {
        //std::cout << "找到了loadlibrarya/getprocaddress/virtualalloc 3个地址" << std::endl;
        //转换为真正的函数
        my_g_p_a = (myGetProcAddress_source)myGetProcAddress_addr;
        my_l_l = (myLoadLibraryA_source)myLoadLibraryA_addr;
        my_v_a = (myVirtualAlloc_source)myVirtualAlloc_addr;
        m_set = (mymemset_source)mymemset_addr;
        m_cpy_s = (mymemcpy_s_source)mymemcpy_s_addr;
        m_g_p_id = (myGetCurrentProcessId_soruce)myGetCurrentProcessId_addr; //未使用

    }
    //return 0;


    //std::cout << dll_address << std::endl;

    //解释PE
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)dll_address;
    //std::cout << std::hex << dos->e_magic << std::endl;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((CHAR*)dll_address + dos->e_lfanew);
    //std::cout << std::hex << nt->Signature << std::endl;
    IMAGE_SECTION_HEADER* se = (IMAGE_SECTION_HEADER*)(
        (char*)dll_address +
        dos->e_lfanew +
        nt->FileHeader.SizeOfOptionalHeader +
        sizeof(nt->Signature) +
        sizeof(nt->FileHeader));

    //新申请内存用于存放DLL
    LPVOID new_dll_address = my_v_a(NULL, nt->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (new_dll_address == NULL) {
        //std::cout << "申请空间失败" << std::endl;
        return 0;
    }
    //清0， 简化下面复制节时的操作
    m_set(new_dll_address, 0, nt->OptionalHeader.SizeOfImage);
    //复制dll进申请的空间并内存对齐
    //先复制头部
    m_cpy_s(new_dll_address, nt->OptionalHeader.SizeOfImage, (char*)dll_address, nt->OptionalHeader.SizeOfHeaders);
    //一个节一个节复制
    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        if (se[i].Misc.VirtualSize == 0) continue;
        //计算内存大小与文件大小
        DWORD copy_size = min(se[i].SizeOfRawData, se[i].Misc.VirtualSize);//取小的一边， 是从文件复制进内存， 取值要在文件里取到内存里去
        //先计算放到哪个起始位置
        void* mem_start = (char*)new_dll_address + se[i].VirtualAddress;//获取内存偏移起始地址
        void* file_source = (char*)dll_address + se[i].PointerToRawData;//获取节在文件中的起始位置
        m_cpy_s(mem_start, copy_size, file_source, copy_size);
        //std::cout << "第" << i << "个节复制完成" << std::endl;

    }
    //std::cout << "所有节复制完成" << std::endl;

    //修复重定位表
    if ((ULONGLONG)new_dll_address != nt->OptionalHeader.ImageBase) {
        //std::cout << "重定位表要修复" << std::endl;
        //是否存在重定位表
        IMAGE_DATA_DIRECTORY data = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

        if (data.VirtualAddress == 0 || data.Size == 0) {
            // 没有重定位表
        }
        else {
            //计算分配到的基地址与预期的差值 
            ULONGLONG delate = (ULONGLONG)new_dll_address - nt->OptionalHeader.ImageBase;
            // 有重定位表
            //修复工作
            IMAGE_BASE_RELOCATION* reloc = (PIMAGE_BASE_RELOCATION)((byte*)new_dll_address + data.VirtualAddress);//找到表
            while (reloc->SizeOfBlock != 0) {

                //std::cout << "重定位表地址偏移: " << reloc->VirtualAddress << std::endl;
                //std::cout << "重定位表大小: " << reloc->SizeOfBlock << std::endl;
                //修复工作
                WORD* typeoffset = (WORD*)((char*)reloc + 8);//获取WORD起始地址
                WORD number_typeoffset = (reloc->SizeOfBlock - 8) / 2;
                for (int i = 0; i < number_typeoffset; i++) {
                    WORD type = (*typeoffset >> 12) & 0xF;   // 高4位, 用于修复的类型
                    WORD offset = *typeoffset & 0x0FFF;        // 低12位， 表示要修复的地址离reloc的偏移
                    DWORD target_address = reloc->VirtualAddress + offset;//要修复的地址
                    //std::cout << "高4位为:" << type << std::endl;
                    //std::cout << "低12位为:" << offset << std::endl;
                    //std::cout << "修复的地址为:" << target_address << std::endl;

                    //判断地址并修复
                    if (type == 0xa) {//64
                        //std::cout << "64位修复 - 目前加载地址与imagebase相差为:" << delate << std::endl;
                        ULONGLONG* real_fix_address = (ULONGLONG*)((char*)new_dll_address + target_address);
                        *real_fix_address = *real_fix_address + delate;
                    }
                    if (type == 0x3) {//32
                        //std::cout << "32位修复 - 目前加载地址与imagebase相差为:" << delate << std::endl;
                        DWORD* fix_addr = (DWORD*)((BYTE*)new_dll_address + target_address);//32地址
                        //先找到原本要修复的地址
                        *fix_addr = *fix_addr + (DWORD)delate;//把指针指向的值改变
                    }
                    //std::cout << "修复完成" << std::endl;
                    typeoffset++;//跳到下一个要修复的WORD数组
                }

                //跳到下一个结构
                reloc = (PIMAGE_BASE_RELOCATION)((char*)reloc + reloc->SizeOfBlock);
            }

        }
    }


    //修复IAT
    IMAGE_DATA_DIRECTORY data = nt->OptionalHeader.DataDirectory[1];//导入表
    if (data.Size == 0) {

    }
    else {

        //std::cout << "导入表地址: " << data.VirtualAddress << std::endl; return 0;
        IMAGE_IMPORT_DESCRIPTOR* my_import = (IMAGE_IMPORT_DESCRIPTOR*)(data.VirtualAddress + (char*)new_dll_address);//
        //上面写位了导入表
        while (my_import->Name) {

            //std::cout << my_import->Name << std::endl;
            uintptr_t post = my_import->Name;
            //std::cout << "post:" << post << std::endl;
            //std::cout << (char*)new_dll_address + post << std::endl;
            //return 0;
            //查看是序号还是名称导入
            IMAGE_THUNK_DATA* _int = (IMAGE_THUNK_DATA*)(my_import->OriginalFirstThunk + (char*)new_dll_address);
            //定义iat表， 用于后面修复地址
            IMAGE_THUNK_DATA* _iat = (IMAGE_THUNK_DATA*)(my_import->FirstThunk + (char*)new_dll_address);
            for (;;) {

                void* function_address;
                if (_int->u1.AddressOfData == 0) break;
                if (_int->u1.Ordinal & IMAGE_ORDINAL_FLAG64) {//序号
                    //std::cout << "序号导入" << std::endl;
                    //提取低16位
                    DWORD number = (_int->u1.Ordinal & 0b1111111111111111);//取低十六位
                    //std::cout << "函数序号不是偏移，直接打印，函数序号为:" << number << std::endl;
                    //std::cout << "函数地址(未修复的):" << _iat->u1.Function << std::endl;
                    HMODULE dll = my_l_l((char*)new_dll_address + my_import->Name);
                    function_address = (void*)my_g_p_a(dll, (LPCSTR)number);
                }
                else {
                    //std::cout << "名称导入" << std::endl;
                    IMAGE_IMPORT_BY_NAME* function_name = (IMAGE_IMPORT_BY_NAME*)(_int->u1.AddressOfData + (BYTE*)new_dll_address);
                    //std::cout << function_name->Name << std::endl;

                    //std::cout << "函数地址(未修复的):" << _iat->u1.Function << std::endl;
                    //要修复的话， 要用func_addr = GetProcAddress(hModule, (LPCSTR)importByName->Name);
                    //先加载当前DLL,再查找函数， 最后修改iat中的function地址
                    //iat->u1.Function = (uintptr_t)func_addr;修复
                    HMODULE dll = my_l_l((char*)new_dll_address + my_import->Name);
                    function_address = (void*)my_g_p_a(dll, function_name->Name);


                }
                //
                //_iat->u1.Function = 修复的地址
                _iat->u1.Function = (uintptr_t)function_address;
                //std::cout << "修复后的函数地址为:" << function_address << std::endl;
                _int++;
                _iat++;
            }


            my_import++;

        }
    }



    //返回EOP, 用于执行DLL
    char* eop = (char*)new_dll_address + nt->OptionalHeader.AddressOfEntryPoint;
    //DllMain原型
    using _DllMain = BOOL(WINAPI*) (
        HINSTANCE hinstDLL,  // DLL 模块句柄
        DWORD     fdwReason, // 调用原因（进程/线程附加或分离）
        LPVOID    lpvReserved // 保留参数
        );
    _DllMain dllmain = (_DllMain)eop;
    dllmain(NULL, 1, NULL);
    return 0;
}


//定义一个函数，用于对比字符串， 只对比IAT表中的函数， 所以直接定义成ASCII对比的
__forceinline int my_ascii_cmp(const char* sour, const  char* desc) { //区分大小写
    int sour_len = 0;
    for (; sour[sour_len] != '\0'; sour_len++) {
    }
    //printf("sour字符串长度为:%d\n", sour_len);
    int desc_size = 0;
    for (; desc[desc_size] != '\0'; desc_size++) {
    }
    //printf("desc字符串长度为:%d\n", desc_size);
    //#####################################################
    if (sour_len != desc_size) return 0;
    int i = 0;
    for (; sour[i] == desc[i]; i++) {
        if (i == sour_len) break;
    }
    if (sour_len != i) return 0;

    return 1;
}

//用于查找特定函数
__forceinline ULONGLONG* my_check_function_address(myLDR_DATA_TABLE_ENTRY* table_entry, ULONGLONG dll_base, char* check_function_name) {
    //用来查找所有DLL中的两个函数， 一个是LoadLibraryA,另一个是GetProcAddress
    //最后一个为当前进程的exe程序，还没跳过
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)table_entry->DllBase;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((char*)table_entry->DllBase + dos->e_lfanew);

    IMAGE_DATA_DIRECTORY data = nt->OptionalHeader.DataDirectory[0];
    if (data.Size == 0) {
        //printf("当前dll没有导出表\n");
    }
    else {
        //printf("当前dll的导出表大小为:%d\n", data.Size);
        //当存在导出表时， 编历导出表并查找函数地址
        IMAGE_EXPORT_DIRECTORY* my_export = (IMAGE_EXPORT_DIRECTORY*)((char*)dll_base + data.VirtualAddress);
        //char* iat_dll_name = new char[100] {};
        //iat_dll_name = my_export->Name + (char*)dll_base;
        //printf("Dll名字是:%s\n", iat_dll_name);//当前DLL名字
        for (int i = 0; i < my_export->NumberOfNames; i++) {
            DWORD* iat_table = (DWORD*)(my_export->AddressOfNames + (char*)dll_base);
            WORD* iat_ordinals = (WORD*)(my_export->AddressOfNameOrdinals + (char*)dll_base);
            DWORD* iat_address = (DWORD*)(my_export->AddressOfFunctions + (char*)dll_base);
            //

            char* iat_function_name = (char*)(iat_table[i] + (char*)dll_base);
            //printf("%d函数名字是:[%s]\n",i, iat_function_name);
            //printf("要查找的函数是:[%s]\n", check_function_name);
            if (my_ascii_cmp(check_function_name, iat_function_name)) {
                //printf("找到函数了-----------------------------------%s\n", check_function_name);
                DWORD ordinals = iat_ordinals[i];//获取对应下标
                ULONGLONG* function_address = (ULONGLONG*)(iat_address[ordinals] + (char*)dll_base);//偏移求得地址
                return function_address;
            }
        }
    }
    return 0;
}


//内存偏移转文件偏移
//第一个参数是为了编历节， 第二个参数是为了获取节的大小，第三个数参为内存偏移
ULONGLONG mem_to_file(IMAGE_SECTION_HEADER* se_head, IMAGE_NT_HEADERS* nt_head, ULONGLONG address) {
    //打印节里的内存地址
    DWORD file_post{};//用于返回地址
    for (int i = 0; i < nt_head->FileHeader.NumberOfSections; i++) {
        //printf("-------------------------------\n");
        BYTE name[9]{};
        memcpy_s(name, 9, se_head[i].Name, 8);
        //printf("当前节的名字是:%s\n", name);
        DWORD vir_size = se_head[i].Misc.VirtualSize;
        //printf("当前节在内存中的大小:%x\n", vir_size);
        //printf("当前节在内存中的偏移地址:%x\n", se_head[i].VirtualAddress);
        //printf("当前节所属内存范围:%x - %x \n", se_head[i].VirtualAddress, se_head[i].VirtualAddress + vir_size);
        //printf("当前节所对应的文件偏移为:%x\n", se_head[i].PointerToRawData);
        DWORD se_virtualaddress_start = se_head[i].VirtualAddress;
        DWORD se_virtualaddress_end = se_head[i].VirtualAddress + vir_size;
        if ((se_virtualaddress_start <= address) && (address < se_virtualaddress_end)) {
            //printf("IMAGE_DATA_DIRECTORY 中的内存地址在这个节的内存地址范围里\n");
            DWORD virtual_post = address - se_virtualaddress_start;
            //printf("IMAGE_DATA_DIRECTORY 中的内存地址离%x 的距离为:%x\n", se_virtualaddress_start, virtual_post);
            file_post = se_head[i].PointerToRawData + virtual_post;//转换成功的文件偏移
            //printf("IMAGE_DATA_DIRECTORY 的文件偏移为:%x + %x =  %x\n", se_head[i].PointerToRawData, virtual_post, file_post);
            return file_post;
        }

    }
    //return 0;
}