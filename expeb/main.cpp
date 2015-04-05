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
#endif

#ifdef DEBUG
#include <stdio.h>
#define LOGDEBUG printf
#else
#define LOGDEBUG
#endif

static struct {
    void *ntdll;
    void *kernel32;
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

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa813708(v=vs.85).aspx
struct list_entry {
   struct list_entry *Flink;
   struct list_entry *Blink;
};

typedef struct _PEB_LDR_DATA {
  unsigned char  Reserved1[8];
  void          *Reserved2[3];
  list_entry     InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LSA_UNICODE_STRING {
  unsigned short  Length;
  unsigned short  MaximumLength;
  unsigned short *Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY {
    void * Reserved1[2];
    LIST_ENTRY InMemoryOrderLinks;
    void * Reserved2[2];
    void * DllBase;
    void * EntryPoint;
    void * Reserved3;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    /* not used */
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa813706(v=vs.85).aspx
typedef struct _PEB {
  unsigned char  Reserved1[2];
  unsigned char  BeingDebugged;
  unsigned char  Reserved2[1];
  void          *Reserved3[2];
  PPEB_LDR_DATA  Ldr;
  void          *ProcessParameters;
  unsigned char  Reserved4[104];
  void          *Reserved5[52];
  void          *PostProcessInitRoutine;
  unsigned char  Reserved6[128];
  void          *Reserved7[1];
  unsigned long  SessionId;
} PEB, *PPEB;

// copied from ntdef.h
#define containing_record(address, type, field) ((type *)((unsigned char *)(address) - (long)(&((type *)0)->field)))

void GetProcAddress(void *ImageBase) {
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

                LOGDEBUG("%4d->%4d %p %p {0x%08x, 0x%08x, ..} %s\n",
                    i, Ordinals[i], Name, Function,
                    Name[0], Name[1], (const char *)Name);
            }
        }
    }
}

void GetImageBase(PPEB Peb) {
    list_entry *begin = &Peb->Ldr->InMemoryOrderModuleList;
    for ( list_entry *p = begin->Flink ; p!=begin ; p=p->Flink ) {
        PLDR_DATA_TABLE_ENTRY entry = containing_record(p, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        unsigned int *Name = (unsigned int *)entry->BaseDllName.Buffer;
        if ( !g.ntdll &&
             (Name[0]==0x0074006e && Name[1]==0x006c0064 && Name[2]==0x002e006c) ||
             (Name[0]==0x0054004e && Name[1]==0x004c0044 && Name[2]==0x002e004c) ) {
            g.ntdll = entry->DllBase;
        }
        else if ( !g.kernel32 &&
                  (Name[0]==0x0045004b && Name[1]==0x004e0052 && Name[2]==0x004c0045 && Name[3]==0x00320033 && Name[4]==0x0044002e) ||
                  (Name[0]==0x0065006b && Name[1]==0x006e0072 && Name[2]==0x006c0065 && Name[3]==0x00320033 && Name[4]==0x0064002e) ) {
            g.kernel32 = entry->DllBase;
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
    void *Peb = NULL;

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

int main() {
    GetImageBase(GetPeb());
    GetProcAddress(g.ntdll);
    GetProcAddress(g.kernel32);
    return 0;
}
