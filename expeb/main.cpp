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
#endif

#include <stdio.h>

static struct {
    void *LoadLibrary;
    void *FreeLibrary;
    void *GetProcAddress;
    void *CreateThread;
    void *ExitThread;
    void *WaitForSingleObject;
    void *CloseHandle;
} g = {0};

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

void GetProcAddress(unsigned long long int ImageBase) {
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

                if ( !g.LoadLibrary && Name[0]==0x64616f4c && Name[1]==0x7262694c && Name[2]==0x57797261 && (Name[3]&0xff)==0 ) {
                    g.LoadLibrary = Function;
                }
                else if ( !g.FreeLibrary && Name[0]==0x65657246 && Name[1]==0x7262694c && Name[2]==0x00797261 ) {
                    g.FreeLibrary = Function;
                }
                else if ( !g.GetProcAddress && Name[0]==0x50746547 && Name[1]==0x41636f72 && Name[2]==0x65726464 && (Name[3]&0xffffff)==0x007373 ) {
                    g.GetProcAddress = Function;
                }
                else if ( !g.CreateThread && Name[0]==0x61657243 && Name[1]==0x68546574 && Name[2]==0x64616572 && (Name[3]&0xff)==0 ) {
                    g.CreateThread = Function;
                }
                else if ( !g.ExitThread && Name[0]==0x456c7452 && Name[1]==0x55746978 && Name[2]==0x54726573 && Name[3]==0x61657268 && (Name[4]&0xffff)==0x0064 ) {
                    g.ExitThread = Function;
                }
                else if ( !g.WaitForSingleObject && Name[0]==0x74696157 && Name[1]==0x53726f46 && Name[2]==0x6c676e69 && Name[3]==0x6a624f65 && Name[4]==0x00746365 ) {
                    g.WaitForSingleObject = Function;
                }
                else if ( !g.CloseHandle && Name[0]==0x736f6c43 && Name[1]==0x6e614865 && Name[2]==0x00656c64 ) {
                    g.CloseHandle = Function;
                }
                else {
                    continue;
                }

                printf("%4d->%4d %p %p { 0x%08x 0x%08x .. } %s\n",
                    i, Ordinals[i], Name, Function,
                    Name[0], Name[1], Name);
            }
        }
    }
}

int main() {
#ifdef MSVC
    GetProcAddress((unsigned long long int)GetModuleHandle(L"ntdll.dll"));
    GetProcAddress((unsigned long long int)GetModuleHandle(L"kernel32.dll"));
#endif

    printf("%p KERNEL32!CloseHandle\n", g.CloseHandle);
    printf("%p KERNEL32!WaitForSingleObject\n", g.WaitForSingleObject);
    printf("%p KERNEL32!CreateThreadStub\n", g.CreateThread);
    printf("%p ntdll!RtlExitUserThread\n", g.ExitThread);
    printf("%p KERNEL32!FreeLibraryStub\n", g.FreeLibrary);
    printf("%p KERNEL32!GetProcAddressStub\n", g.GetProcAddress);
    printf("%p KERNEL32!LoadLibraryW\n", g.LoadLibrary);

    return 0;
}
