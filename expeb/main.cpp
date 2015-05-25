//
// main.cpp
//
// https://msdn.microsoft.com/en-us/magazine/cc301808.aspx
//

#ifdef _WIN32
#define MSVC
#endif

#ifdef MSVC
#include <windows.h>
#include <intrin.h>

#define MEM_RELEASE 0x8000
#else
#define WINAPI
#endif

#ifdef DEBUG
#include <stdio.h>
#define LOGDEBUG printf
#else
#define LOGDEBUG
#endif

#include "../common.h"

struct image_export_directory {
    unsigned int   Characteristics;
    unsigned int   TimeDateStamp;
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    unsigned int   Name;
    unsigned int   Base;
    unsigned int   NumberOfFunctions;
    unsigned int   NumberOfNames;
    unsigned int   AddressOfFunctions;
    unsigned int   AddressOfNames;
    unsigned int   AddressOfNameOrdinals;
};

#define image_file_machine_i386  0x014c
#define image_file_machine_amd64 0x8664

// copied from ntdef.h
#define containing_record(address, type, field) ((type *)((unsigned char *)(address) - (long)(&((type *)0)->field)))

void GetProcAddress(void *ImageBase, Package *package) {
    const unsigned short MZ_SIGNATURE = 0x5a4d;
    const unsigned int PE_SIGNATURE = 0x4550;

    unsigned char *p = (unsigned char *)ImageBase;
    if ( *(unsigned short *)p == MZ_SIGNATURE ) {
        // 0x3c = _IMAGE_DOS_HEADER::e_lfanew
        unsigned int Offset = *(unsigned int *)(p + 0x3c);
        if ( *(unsigned int *)(p + Offset)==PE_SIGNATURE ) {
            // 0x04 = ntdll!_IMAGE_NT_HEADERS::FileHeader::Machine
            unsigned short Platform = *(unsigned short *)(p + Offset + 0x04);
            if ( Platform!=image_file_machine_i386 && Platform!=image_file_machine_amd64 ) {
                return;
            }
            // 0x18 = ntdll!_IMAGE_NT_HEADERS::OptionalHeader
            unsigned int Rva_Exports = *(unsigned int *)(p + Offset + 0x18 + (Platform==image_file_machine_i386 ? 0x60 : 0x70));
            image_export_directory *ExportDirectory = (image_export_directory*)(p + Rva_Exports);

            unsigned int * Names = (unsigned int *)(p + ExportDirectory->AddressOfNames);
            unsigned short * Ordinals = (unsigned short *)(p + ExportDirectory->AddressOfNameOrdinals);
            unsigned int * Functions = (unsigned int *)(p + ExportDirectory->AddressOfFunctions);
            for ( unsigned int i=0 ; i<ExportDirectory->NumberOfNames ; ++i ) {
                unsigned int * Name = (unsigned int *)(p + Names[i]);
                void *Function = p + Functions[Ordinals[i]];

                if ( !package->xxxLoadLibrary && Name[0]==0x64616f4c && Name[1]==0x7262694c && Name[2]==0x57797261 && (Name[3]&0xff)==0 ) {
                    package->xxxLoadLibrary = Function;
                }
                else if ( !package->xxxFreeLibrary && Name[0]==0x65657246 && Name[1]==0x7262694c && Name[2]==0x00797261 ) {
                    package->xxxFreeLibrary = Function;
                }
                else if ( !package->xxxGetProcAddress && Name[0]==0x50746547 && Name[1]==0x41636f72 && Name[2]==0x65726464 && (Name[3]&0xffffff)==0x007373 ) {
                    package->xxxGetProcAddress = Function;
                }
                else {
                    continue;
                }

                LOGDEBUG("%4d->%4d %p %p {0x%08x, 0x%08x, ..} %s\n",
                    i, Ordinals[i], Name, Function,
                    Name[0], Name[1], (const char *)Name);
            }
        }
    }
}

void GetImageBase(Package *package) {
    pj_list_entry *begin = &package->peb->Ldr->InMemoryOrderModuleList;
    for ( pj_list_entry *p = begin->Flink ; p!=begin ; p=p->Flink ) {
        PLDR_DATA_TABLE_ENTRY entry = containing_record(p, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        unsigned int *Name = (unsigned int *)entry->BaseDllName.Buffer;
        if ( !package->kernel32 &&
                  (Name[0]==0x0045004b && Name[1]==0x004e0052 && Name[2]==0x004c0045 && Name[3]==0x00320033 && (Name[4]&0xffff)==0x002e) ||
                  (Name[0]==0x0065006b && Name[1]==0x006e0072 && Name[2]==0x006c0065 && Name[3]==0x00320033 && (Name[4]&0xffff)==0x002e) ) {
            package->kernel32 = entry->DllBase;
        }
        else {
            continue;
        }

        LOGDEBUG("%p %S\n", entry->DllBase, entry->BaseDllName.Buffer);
    }
}

// http://en.wikipedia.org/wiki/Win32_Thread_Information_Block
//
// [fs:0030h] --> x86 PEB
// [gs:0030h] --> x64 TEB
PPEB GetPeb() {
    void *Peb = 0;

#ifdef MSVC
#ifdef _WIN64
    void *Teb = (void *)__readgsqword(0x30);
    LOGDEBUG("TEB = %p\n", Teb);

    // 0x60 = ntdll!_TEB::ProcessEnvironmentBlock
    Peb = *(void**)((unsigned char *)Teb + 0x60);
#else
    Peb = (void *)__readfsdword(0x30);
#endif
#endif

    LOGDEBUG("PEB = %p\n", Peb);
    return (PPEB)Peb;
}

unsigned int WINAPI ShellCode(Package *p) {
    unsigned long long ll1 = 0x123;
    unsigned long long ll2 = 0x456;
    unsigned int Ret = 0;
    void *f;

    p->peb = GetPeb();
    GetImageBase(p);
    GetProcAddress(p->kernel32, p);

    void *hm = LOADLIBRARY(p->xxxLoadLibrary)(p->DllPath);
    if ( hm ) {
        f = GETPROCADDRESS(p->xxxGetProcAddress)(hm, MAKEINTRESOURCEA(0xdead));
        if ( f ) {
            Ret = THREADPROC(f)(p);
        }
        FREELIBRARY(p->xxxFreeLibrary)(hm);
    }

    f = GETPROCADDRESS(p->xxxGetProcAddress)(p->kernel32, &ll1);
    if ( f ) {
        Ret = VIRTUALFREE(f)(p, 0, MEM_RELEASE);
    }

    f = GETPROCADDRESS(p->xxxGetProcAddress)(p->kernel32, &ll2);
    if ( f ) {
        (EXITTHREAD(f))(Ret);
    }

    return Ret;
}

int main() {
    Package p = {0};
    LOGDEBUG("sizeof(Package) = %lu\n", sizeof(Package));
    ShellCode(&p);
    return 0;
}
