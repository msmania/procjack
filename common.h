//
// common.h
//

#pragma once

// https://msdn.microsoft.com/en-us/library/windows/desktop/aa813708(v=vs.85).aspx
typedef struct _pj_list_entry {
   struct _pj_list_entry *Flink;
   struct _pj_list_entry *Blink;
} pj_list_entry;

typedef struct _PEB_LDR_DATA {
  unsigned char  Reserved1[8];
  void          *Reserved2[3];
  pj_list_entry  InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LSA_UNICODE_STRING {
  unsigned short  Length;
  unsigned short  MaximumLength;
  unsigned short *Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY {
    void           *Reserved1[2];
    pj_list_entry   InMemoryOrderLinks;
    void           *Reserved2[2];
    void           *DllBase;
    void           *EntryPoint;
    void           *Reserved3;
    UNICODE_STRING  FullDllName;
    UNICODE_STRING  BaseDllName;
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

struct Package {
    unsigned char InitialCode[2048];
    unsigned short DllPath[260];
    PPEB  peb;
    void *kernel32;
    void *xxxLoadLibrary;
    void *xxxFreeLibrary;
    void *xxxGetProcAddress;
    unsigned char Context[1];
};

typedef void *(WINAPI *LOADLIBRARY)(
  _In_  void *lpFileName
);

typedef void *(WINAPI *GETPROCADDRESS)(
  _In_  void *hModule,
  _In_  void *lpProcName
);

typedef void *(WINAPI *CREATETHREAD)(
  void *lpThreadAttributes,
  size_t dwStackSize,
  void *lpStartAddress,
  void *lpParameter,
  unsigned int dwCreationFlags,
  void *lpThreadId
);

typedef unsigned int (WINAPI *WAITFORSINGLEOBJECT)(
  void *hHandle,
  unsigned int dwMilliseconds
);

typedef unsigned int (WINAPI *CLOSEHANDLE)(
  void *hObject
);

typedef unsigned int (WINAPI *FREELIBRARY)(
  void *hModule
);

typedef void (WINAPI *EXITTHREAD)(
  unsigned int dwExitCode
);

typedef unsigned int (WINAPI *THREADPROC)(
  void *lpParameter
);

typedef unsigned int (WINAPI *VIRTUALFREE)(
  void *lpAddress,
  size_t dwSize,
  unsigned int dwFreeType
);
