#include <string>
#include <windows.h>
#include "../common.h"
#include "utils.h"
#include "pkg_creator.h"

#define LOGERROR wprintf
#define LOGINFO LOGERROR

std::string to_utf8(const wchar_t *utf16);
std::string build_optional_args(int argc, wchar_t *argv[]);
bool DisablePolicy(HANDLE target);

void Inject(DWORD RemoteProcessId,
            LPCWSTR FilenameToInject,
            INT Ordinal,
            const std::string &args,
            bool Wait,
            bool TryToDisablePolicy) {
  constexpr DWORD DesiredAccess = PROCESS_VM_OPERATION        // VirtualAllocEx
                                  | PROCESS_QUERY_INFORMATION // IsWow64Process
                                  | PROCESS_VM_WRITE          // WriteProcessMemory
                                  | PROCESS_CREATE_THREAD     // CreateThread
                                  | PROCESS_SET_INFORMATION;  // NtSetInformationProcess
  constexpr WCHAR PLATFORM_LABEL[][10] = { L"WIN32", L"WIN64", L"WOW64" };

  PackageCreator package;
  HANDLE TargetProcess = nullptr;
  PVOID RemoteAddress = nullptr;
  HANDLE RemoteThread = nullptr;
  DWORD RemoteThreadId = 0;
  DWORD WaitResult = 0;
  SYSTEM_INFO si;
  enum PLATFORM { win32, win64, wow64 } Platform = win32;

  TargetProcess = OpenProcess(DesiredAccess, FALSE, RemoteProcessId);
  if (!TargetProcess) {
    LOGERROR(L"OpenProcess failed - %08x\n", GetLastError());
    goto cleanup;
  }

  ZeroMemory(&si, sizeof(si));
  GetNativeSystemInfo(&si);
  if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
    BOOL IsWow64 = FALSE;
    if (!IsWow64Process(TargetProcess, &IsWow64)) {
      LOGERROR(L"IsWow64Process failed - %08x\n", GetLastError());
      goto cleanup;
    }
    Platform = IsWow64 ? wow64 : win64;
  }
  else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
    Platform = win32;
  }
  else {
    LOGERROR(L"Unsupported platform.\n");
    goto cleanup;
  }

  if (TryToDisablePolicy
      && DisablePolicy(TargetProcess)) {
    LOGINFO(L"ProcessMitigationPolicy has been successfully disabled!\n");
  }

  if (!package.Fill(Platform == win64, FilenameToInject, Ordinal, args)) {
    goto cleanup;
  }

  RemoteAddress = package.Inject(TargetProcess);
  if (!RemoteAddress) goto cleanup;

  RemoteThread = CreateRemoteThread(
    TargetProcess,
    /*lpThreadAttributes*/nullptr,
    /*dwStackSize*/0,
    reinterpret_cast<LPTHREAD_START_ROUTINE>(RemoteAddress),
    RemoteAddress,
    /*dwCreationFlags*/0,
    &RemoteThreadId);
  if (!RemoteThread) {
    LOGERROR(L"CreateRemoteThread failed - %08x\n", GetLastError());
    goto cleanup;
  }

  LOGINFO(L"Hijacking: PID=%04x (%s) TID=%04x VM=%p\n",
    RemoteProcessId,
    PLATFORM_LABEL[Platform],
    RemoteThreadId,
    RemoteAddress);

  if (Wait) {
    WaitResult = WaitForSingleObject(RemoteThread, INFINITE);
    if (WaitResult != WAIT_OBJECT_0) {
      LOGERROR(L"WaitForSingleObject failed - %08x\n", WaitResult);
      goto cleanup;
    }
  }

cleanup:
  if (RemoteThread) CloseHandle(RemoteThread);
  if (RemoteAddress && Wait) {
    // VirtualFreeEx won't cause double-free if the page is freed by the injected code.
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa366892.aspx
    VirtualFreeEx(TargetProcess, RemoteAddress, 0, MEM_RELEASE);
  }
  if (TargetProcess) CloseHandle(TargetProcess);
}

int wmain(int argc, wchar_t *argv[]) {
  if (argc < 3) {
    LOGINFO(L"\nUSAGE: pj.exe [-d] [-w] <PID> <FILE>[?ORDINAL] [ARGS]\n"
            L"  -d         Try to disable ProcessMitigationPolicy before injection\n"
            L"  -w         Keep pj.exe running until the injected thread is signaled\n"
            L"  PID        PID of a target process to inject into\n"
            L"  FILE       DLL or a flat binary to inject\n"
            L"  ORDINAL    Orginal# of a function exported from the DLL to run in the target\n"
            L"             If not specified, FILE is assumed to be a flat binary.\n"
            L"  ARGS       Optional arguments that can be accessed as Package::args in the injected code\n"
            );
    return 1;
  }
  int arg_index = 1;
  bool disable_policy = _wcsicmp(argv[arg_index], L"-D") == 0;
  if (disable_policy) ++arg_index;
  bool wait = _wcsicmp(argv[arg_index], L"-W") == 0;
  if (wait) ++arg_index;
  auto pair = split<wchar_t>(argv[arg_index + 1], L'?');
  auto optional_args = build_optional_args(argc - arg_index - 2,
                                           &argv[arg_index + 2]);
  Inject(_wtoi(argv[arg_index]),
         pair.first.c_str(),
         pair.second,
         optional_args,
         wait,
         disable_policy);
  return 0;
}
