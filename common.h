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
struct PEB {
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
};

constexpr uint32_t SHELLCODE_CAPACITY = 2048;

// HybridPointer makes sure a pointer consumes 64bit on 32bit platform, so that
// the offset to the `args` is not affected by compiler target platform.  This
// enables 64bit pj.exe to inject 32bit DLL into a WoW64 process.
template<typename T>
union HybridPointer {
  T *p;
  uint64_t dummy;
  T* &operator=(void *value) {
    return p = reinterpret_cast<T*>(value);
  }
  operator T*() {
    return p;
  }
  T* operator->() const {
    return p;
  }
};

struct Package {
  union NonWritable {
    struct {
      uint8_t shellcode[SHELLCODE_CAPACITY];
      wchar_t dllpath[260];
      char args[1];
    };
    uint8_t dummy[4096];
  } nw;

  HybridPointer<PEB> peb;
  HybridPointer<void> kernel32;
  HybridPointer<void*(WINAPI)(void*)> xxxLoadLibrary;
  HybridPointer<uint32_t(WINAPI)(void*)> xxxFreeLibrary;
  HybridPointer<void*(WINAPI)(void*, void*)> xxxGetProcAddress;
};

constexpr uint32_t VALLOC_GRANULARITY = 1 << 16;
static_assert(sizeof(Package) <= VALLOC_GRANULARITY, "Too big Package!");
constexpr uint32_t ARGS_CAPACITY =
  offsetof(Package, peb) - offsetof(Package, nw.args);
