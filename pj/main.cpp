//
// main.cpp
//
// http://www.codeproject.com/Articles/4610/Three-Ways-to-Inject-Your-Code-into-Another-Proces
//

#include <windows.h>
#include <stdio.h>

#define LOGERROR wprintf
#define LOGINFO LOGERROR

#define CONTEXT_CAST(type, pointer, field) ((type)(((CInjectData::Package*)(pointer))->field))

class CInjectData {
private:
    bool FillFunction(BOOL Is64bit, LPCWSTR DllPath, WORD Ordinal, PBYTE Buffer, DWORD Length) {
        HMODULE Kernel32 = GetModuleHandle(L"kernel32.dll");
        FARPROC LoadLibrary = NULL;
        FARPROC FreeLibrary = NULL;
        FARPROC GetProcAddress = NULL;
        FARPROC CreateThread = NULL;
        FARPROC ExitThread = NULL;
        FARPROC WaitForSingleObject = NULL;
        FARPROC CloseHandle = NULL;

        if ( Length<sizeof(Package) ) {
            LOGERROR(L"Buffer is small\n", 0);
            return false;
        }

        Package *p = (Package*)Buffer;

        if ( !GetFullPathName(DllPath, MAX_PATH, (PWCHAR)p->DllPath, NULL) ) {
            LOGERROR(L"GetFullPathName failed - %08x\n", GetLastError());
            return false;
        }

        // We can assume the imagebase of kernel32.dll/ntdll.dll is the same in a remote process.
        // However, we cannot use this trick for WOW process.
        LoadLibrary = ::GetProcAddress(Kernel32, "LoadLibraryW");
        FreeLibrary = ::GetProcAddress(Kernel32, "FreeLibrary");
        GetProcAddress = ::GetProcAddress(Kernel32, "GetProcAddress");
        CreateThread = ::GetProcAddress(Kernel32, "CreateThread");
        ExitThread = ::GetProcAddress(Kernel32, "ExitThread");
        WaitForSingleObject = ::GetProcAddress(Kernel32, "WaitForSingleObject");
        CloseHandle = ::GetProcAddress(Kernel32, "CloseHandle");

        LOGINFO(L"%p KERNEL32!CloseHandle\n", CloseHandle);
        LOGINFO(L"%p KERNEL32!WaitForSingleObject\n", WaitForSingleObject);
        LOGINFO(L"%p KERNEL32!CreateThreadStub\n", CreateThread);
        LOGINFO(L"%p ntdll!RtlExitUserThread\n", ExitThread);
        LOGINFO(L"%p KERNEL32!FreeLibraryStub\n", FreeLibrary);
        LOGINFO(L"%p KERNEL32!GetProcAddressStub\n", GetProcAddress);
        LOGINFO(L"%p KERNEL32!LoadLibraryW\n", LoadLibrary);

        if ( Is64bit ) {
            static BYTE CODE64[] = {
                0x48, 0x89, 0x5c, 0x24, 0x08, 0x57, 0x48, 0x83, 0xec, 0x30, 0x48, 0x89, 0xcb, 0x48, 0x81, 0xc1,
                0x00, 0x04, 0x00, 0x00, 0x48, 0xb8, 0x78, 0x56, 0x34, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0,
                0xba, 0xad, 0xde, 0x00, 0x00, 0x48, 0x89, 0xc1, 0x48, 0x89, 0xc7, 0x48, 0xb8, 0x78, 0x56, 0x34,
                0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x31, 0xc9, 0x49, 0x89, 0xd9, 0x48, 0x89, 0x4c, 0x24,
                0x28, 0x49, 0x89, 0xc0, 0x31, 0xd2, 0x89, 0x4c, 0x24, 0x20, 0x48, 0xb8, 0x78, 0x56, 0x34, 0x12,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x83, 0xca, 0xff, 0x48, 0x89, 0xc1, 0x48, 0x89, 0xc3, 0x48,
                0xb8, 0x78, 0x56, 0x34, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x48, 0x89, 0xd9, 0x48, 0xb8,
                0x78, 0x56, 0x34, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x48, 0x89, 0xf9, 0x48, 0xb8, 0x78,
                0x56, 0x34, 0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x31, 0xc9, 0x48, 0xb8, 0x78, 0x56, 0x34,
                0x12, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0xcc, 0xcc
            };
            *(PULONG64)&(((PBYTE)CODE64)[0x16]) = (ULONG64)LoadLibrary;
            *(PULONG64)&(((PBYTE)CODE64)[0x2d]) = (ULONG64)GetProcAddress;
            *(PULONG64)&(((PBYTE)CODE64)[0x4c]) = (ULONG64)CreateThread;
            *(PULONG64)&(((PBYTE)CODE64)[0x61]) = (ULONG64)WaitForSingleObject;
            *(PULONG64)&(((PBYTE)CODE64)[0x70]) = (ULONG64)CloseHandle;
            *(PULONG64)&(((PBYTE)CODE64)[0x7f]) = (ULONG64)FreeLibrary;
            *(PULONG64)&(((PBYTE)CODE64)[0x8d]) = (ULONG64)ExitThread;
            *(PDWORD)&(((PBYTE)CODE64)[0x21]) = (DWORD)Ordinal;

            CopyMemory(p->InitialCode, CODE64, sizeof(CODE64));
        }
        else {
            static BYTE CODE32[] = {
                0x56, 0x8b, 0x74, 0x24, 0x08, 0x57, 0x8d, 0x86, 0x00, 0x04, 0x00, 0x00, 0x50, 0xb8, 0x78, 0x56,
                0x34, 0x12, 0xff, 0xd0, 0x89, 0xc7, 0x68, 0xad, 0xde, 0x00, 0x00, 0x57, 0xb8, 0x78, 0x56, 0x34,
                0x12, 0xff, 0xd0, 0x6a, 0x00, 0x6a, 0x00, 0x56, 0x50, 0x6a, 0x00, 0x6a, 0x00, 0xb8, 0x78, 0x56,
                0x34, 0x12, 0xff, 0xd0, 0x89, 0xc6, 0x6a, 0xff, 0x56, 0xb8, 0x78, 0x56, 0x34, 0x12, 0xff, 0xd0,
                0x56, 0xb8, 0x78, 0x56, 0x34, 0x12, 0xff, 0xd0, 0x57, 0xb8, 0x78, 0x56, 0x34, 0x12, 0xff, 0xd0,
                0x6a, 0x00, 0xb8, 0x78, 0x56, 0x34, 0x12, 0xff, 0xd0, 0x5f, 0x5e, 0xcc, 0xcc
            };
            *(PULONG32)&(((PBYTE)CODE32)[0x0e]) = (ULONG32)LoadLibrary;
            *(PULONG32)&(((PBYTE)CODE32)[0x1d]) = (ULONG32)GetProcAddress;
            *(PULONG32)&(((PBYTE)CODE32)[0x2e]) = (ULONG32)CreateThread;
            *(PULONG32)&(((PBYTE)CODE32)[0x3a]) = (ULONG32)WaitForSingleObject;
            *(PULONG32)&(((PBYTE)CODE32)[0x42]) = (ULONG32)CloseHandle;
            *(PULONG32)&(((PBYTE)CODE32)[0x4a]) = (ULONG32)FreeLibrary;
            *(PULONG32)&(((PBYTE)CODE32)[0x53]) = (ULONG32)ExitThread;
            *(PDWORD)&(((PBYTE)CODE32)[0x17]) = (DWORD)Ordinal;

            CopyMemory(p->InitialCode, CODE32, sizeof(CODE32));
        }

        return true;
    }

    typedef HMODULE (WINAPI *LOADLIBRARY)(
      _In_  LPCWSTR lpFileName
    );

    typedef FARPROC (WINAPI *GETPROCADDRESS)(
      _In_  HMODULE hModule,
      _In_  LPCSTR lpProcName
    );

    typedef HANDLE (WINAPI *CREATETHREAD)(
      _In_opt_   LPSECURITY_ATTRIBUTES lpThreadAttributes,
      _In_       SIZE_T dwStackSize,
      _In_       LPTHREAD_START_ROUTINE lpStartAddress,
      _In_opt_   LPVOID lpParameter,
      _In_       DWORD dwCreationFlags,
      _Out_opt_  LPDWORD lpThreadId
    );

    typedef DWORD (WINAPI *WAITFORSINGLEOBJECT)(
      _In_  HANDLE hHandle,
      _In_  DWORD dwMilliseconds
    );

    typedef BOOL (WINAPI *CLOSEHANDLE)(
      _In_  HANDLE hObject
    );

    typedef BOOL (WINAPI *FREELIBRARY)(
      _In_  HMODULE hModule
    );

    typedef VOID (WINAPI *EXITTHREAD)(
      _In_  DWORD dwExitCode
    );

public:
    struct Package {
        unsigned char InitialCode[2048];
        unsigned short DllPath[260];
        void *peb;
        void *ntdll;
        void *kernel32;
        void *LoadLibrary;
        void *FreeLibrary;
        void *GetProcAddress;
        void *CreateThread;
        void *ExitThread;
        void *WaitForSingleObject;
        void *CloseHandle;
        unsigned char Context[1];
    };

    static DWORD WINAPI ShellCode(_In_  LPVOID p) {
        HMODULE hm = CONTEXT_CAST(LOADLIBRARY, p, LoadLibrary)(CONTEXT_CAST(PWCHAR, p, DllPath));
        if ( hm ) {
            LPTHREAD_START_ROUTINE f = (LPTHREAD_START_ROUTINE)CONTEXT_CAST(GETPROCADDRESS, p, GetProcAddress)(hm, MAKEINTRESOURCEA(0xdead));
            HANDLE ht = CONTEXT_CAST(CREATETHREAD, p, CreateThread)(NULL, 0, f, p, 0, NULL);
            if ( ht ) {
                CONTEXT_CAST(WAITFORSINGLEOBJECT, p, WaitForSingleObject)(ht, INFINITE);
                CONTEXT_CAST(CLOSEHANDLE, p, CloseHandle)(ht);
            }
            CONTEXT_CAST(FREELIBRARY, p, FreeLibrary)(hm);
        }
        CONTEXT_CAST(EXITTHREAD, p, ExitThread)(0);
        return 0;
    }

    CInjectData() {}

    bool FillData(BOOL Is64bit, LPCWSTR FilePath, INT Ordinal, PBYTE Buffer, DWORD Length) {
        bool Ret = false;
        if ( Ordinal>=0 && Ordinal<=0xFFFF ) {
            Ret = FillFunction(Is64bit, FilePath, (WORD)Ordinal, Buffer, Length);
        }
        else {
            HANDLE FileHandle = CreateFile(FilePath,
                GENERIC_READ, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if ( FileHandle==INVALID_HANDLE_VALUE ) {
                LOGERROR(L"CreateFile failed - %08x\n", GetLastError());
            }
            else {
                DWORD BytesWritten = 0;
                if ( ReadFile(FileHandle, Buffer, Length, &BytesWritten, NULL) ) {
                    Ret = true;
                }
                else {
                    LOGERROR(L"ReadFile failed - %08x\n", GetLastError());
                }
                CloseHandle(FileHandle);
            }
        }
        return Ret;
    }
};

void Inject(DWORD RemoteProcessId, LPCWSTR FilenameToInject, INT Ordinal) {

    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms684880(v=vs.85).aspx
    CONST DWORD DesiredAccess =
        PROCESS_VM_OPERATION |      // VirtualAllocEx
        PROCESS_QUERY_INFORMATION | // IsWow64Process
        PROCESS_VM_WRITE |          // WriteProcessMemory
        PROCESS_CREATE_THREAD;      // CreateThread

    CONST DWORD PackageSize = 4096; // 4K should be enough

    HANDLE TargetProcess = NULL;
    PVOID RemoteAddress = NULL;
    BYTE InjectionPackage[PackageSize];
    SIZE_T BytesWritten = 0;
    HANDLE RemoteThread = NULL;
    DWORD RemoteThreadId = 0;
    DWORD WaitResult = 0;

    CInjectData inject;

    SYSTEM_INFO si;
    enum PLATFORM {win32, win64, wow64} Platform = win32;
    CONST WCHAR PLATFORM_LABEL[][10] = { L"WIN32", L"WIN64", L"WOW64" };

    TargetProcess = OpenProcess(DesiredAccess, FALSE, RemoteProcessId);
    if ( !TargetProcess ) {
        LOGERROR(L"OpenProcess failed - %08x\n", GetLastError());
        goto cleanup;
    }

    ZeroMemory(&si, sizeof(si));
    GetNativeSystemInfo(&si);
    if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 ) {
        BOOL IsWow64 = FALSE;
        if ( !IsWow64Process(TargetProcess, &IsWow64) ) {
            LOGERROR(L"IsWow64Process failed - %08x\n", GetLastError());
            goto cleanup;
        }
        Platform = IsWow64 ? wow64 : win64;
    }
    else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL ) {
        Platform = win32;
    }
    else {
        LOGERROR(L"Unsupported platform.\n", 0);
        goto cleanup;
    }

    if ( (sizeof(PVOID)==4 && Platform==win64) ||
         (sizeof(PVOID)==8 && Platform!=win64) ) {
        LOGERROR(L"In the current version, the bitness of target and injector must match.\n", 0);
        goto cleanup;
    }

    ZeroMemory(&InjectionPackage, sizeof(InjectionPackage));
    if ( !inject.FillData(
            Platform==win64,
            FilenameToInject,
            Ordinal,
            InjectionPackage,
            PackageSize) ) {
        goto cleanup;
    }

    CONST WCHAR Spy[] = L"D:\\GitHub\\procjack\\bin64\\spy.dll";
    CopyMemory(CONTEXT_CAST(PVOID, InjectionPackage, DllPath), Spy, sizeof(Spy));

    RemoteAddress = VirtualAllocEx(TargetProcess, NULL, PackageSize,
        MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if ( !RemoteAddress ) {
        LOGERROR(L"VirtualAllocEx failed - %08x\n", GetLastError());
        goto cleanup;
    }

    if ( !WriteProcessMemory(TargetProcess, RemoteAddress, InjectionPackage, PackageSize, &BytesWritten) ) {
        LOGERROR(L"WriteProcessMemory failed - %08x\n", GetLastError());
        goto cleanup;
    }

    RemoteThread = CreateRemoteThread(TargetProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)RemoteAddress, RemoteAddress,
        0, &RemoteThreadId);
    if ( !RemoteThread ) {
        LOGERROR(L"CreateRemoteThread failed - %08x\n", GetLastError());
        goto cleanup;
    }

    LOGINFO(L"Hijacking: PID=%04x (%s) TID=%04x VM=%p\n",
        RemoteProcessId, PLATFORM_LABEL[Platform], RemoteThreadId, RemoteAddress);

    WaitResult = WaitForSingleObject(RemoteThread, INFINITE);
    if ( WaitResult!=WAIT_OBJECT_0 ) {
        LOGERROR(L"WaitForSingleObject failed - %08x\n", WaitResult);
        goto cleanup;
    }

cleanup:
    if ( RemoteThread ) CloseHandle(RemoteThread);
    if ( RemoteAddress ) VirtualFreeEx(TargetProcess, RemoteAddress, 0, MEM_RELEASE);
    if ( TargetProcess ) CloseHandle(TargetProcess);
}

int wmain(int argc, WCHAR *argv[]) {
    if ( argc<0 ) {
        CInjectData::ShellCode(0);
    }
    else if ( argc<3 ) {
        LOGINFO(L"usage: pj <pid> <file> [ordinal#]\n");
    }
    else {
        Inject(_wtoi(argv[1]), argv[2], argc>=4 ? _wtoi(argv[3]) : -1);
    }

    return 0;
}
