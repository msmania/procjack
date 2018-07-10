// https://docs.microsoft.com/en-us/windows/desktop/api/winternl/ns-winternl-_peb_ldr_data
typedef struct _PEB_LDR_DATA {
  unsigned char  Reserved1[8];
  void          *Reserved2[3];
  LIST_ENTRY     InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LSA_UNICODE_STRING {
  unsigned short  Length;
  unsigned short  MaximumLength;
  unsigned short *Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY {
  void           *Reserved1[2];
  LIST_ENTRY      InMemoryOrderLinks;
  void           *Reserved2[2];
  void           *DllBase;
  void           *EntryPoint;
  void           *Reserved3;
  UNICODE_STRING  FullDllName;
  UNICODE_STRING  BaseDllName;
  /* not used */
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

// https://docs.microsoft.com/en-us/windows/desktop/api/winternl/ns-winternl-_peb
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
