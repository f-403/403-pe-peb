#pragma once
#include <windows.h>
#include <winternl.h>

//0x10 bytes (sizeof)
struct _STRING64
{
    USHORT Length;                                                          //0x0
    USHORT MaximumLength;                                                   //0x2
    ULONGLONG Buffer;                                                       //0x8
};
//0x7d0 bytes (sizeof)
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