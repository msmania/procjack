#include <cstdint>
#include <string>
#include <windows.h>
#include <stdio.h>
#include "../common.h"
#include "pkg_creator.h"

#define LOGERROR wprintf
#define LOGINFO LOGERROR

void Inject(DWORD RemoteProcessId,
            LPCWSTR FilenameToInject,
            INT Ordinal,
            const std::string &args,
            bool Wait) {
  constexpr DWORD DesiredAccess = PROCESS_VM_OPERATION        // VirtualAllocEx
                                  | PROCESS_QUERY_INFORMATION // IsWow64Process
                                  | PROCESS_VM_WRITE          // WriteProcessMemory
                                  | PROCESS_CREATE_THREAD;    // CreateThread
  constexpr WCHAR PLATFORM_LABEL[][10] = { L"WIN32", L"WIN64", L"WOW64" };

  PackageCreator package;

  HANDLE TargetProcess = nullptr;
  PVOID RemoteAddress = nullptr;

  SIZE_T BytesWritten = 0;
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

  if (!package.Fill(Platform == win64, FilenameToInject, Ordinal, args)) {
    goto cleanup;
  }

  RemoteAddress = VirtualAllocEx(TargetProcess,
                                 nullptr,
                                 package.Size(),
                                 MEM_RESERVE | MEM_COMMIT,
                                 PAGE_EXECUTE_READWRITE);
  if (!RemoteAddress) {
    LOGERROR(L"VirtualAllocEx failed - %08x\n", GetLastError());
    goto cleanup;
  }

  if (!WriteProcessMemory(TargetProcess,
                          RemoteAddress,
                          package.As<uint8_t>(),
                          package.Size(),
                          &BytesWritten)) {
    LOGERROR(L"WriteProcessMemory failed - %08x\n", GetLastError());
    goto cleanup;
  }

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

template<typename WideCharVersion>
struct xtoi {
  int operator()(const wchar_t *s) {
    return _wtoi(s);
  }
};

template<>
struct xtoi<std::false_type> {
  int operator()(const char *s) {
    return atoi(s);
  }
};

template<typename T>
std::pair<std::basic_string<T>, int> split(const std::basic_string<T> &s,
                                           T delim) {
  std::pair<std::basic_string<T>, int> ret;
  auto pos = s.find(delim);
  if (pos != std::basic_string<T>::npos) {
    ret.first = s.substr(0, pos);
    // xtoi should be faster than stoi
    ret.second = xtoi<std::is_same<wchar_t, T>::type>()(
      s.c_str() + pos + 1);
  }
  else {
    ret.first = s;
    ret.second = -1;
  }
  return ret;
}

#ifdef GTEST
template<typename T>
void pair_checker(const T *input,
                  const T *expected_first,
                  int expected_second) {
  auto pair = split<T>(input, static_cast<T>('?'));
  EXPECT_EQ(pair.first, expected_first);
  EXPECT_EQ(pair.second, expected_second);
};

TEST(string, split) {
  pair_checker<wchar_t>(L"W:\\01.dll?101", L"W:\\01.dll", 101);
  pair_checker<char>("D:\\01.dll?101", "D:\\01.dll", 101);
  pair_checker<char>("D:\\02.dll?102?9", "D:\\02.dll", 102);
  pair_checker<char>("D:\\03.dll?103q9", "D:\\03.dll", 103);
  pair_checker<char>("D:\\04.dll??", "D:\\04.dll", 0);
  pair_checker<char>("D:\\05.dll?abc", "D:\\05.dll", 0);
  pair_checker<char>("D:\\06.dll", "D:\\06.dll", -1);
}
#endif

std::string to_utf8(const wchar_t *utf16) {
  auto required = WideCharToMultiByte(CP_UTF8,
                                     /*dwFlags*/0,
                                     utf16,
                                     /*cchWideChar*/-1,
                                     /*lpMultiByteStr*/nullptr,
                                     0,
                                     /*lpDefaultChar*/nullptr,
                                     /*lpUsedDefaultChar*/nullptr);
  char *utf8 = new char[required + 1];
  WideCharToMultiByte(CP_UTF8,
                      /*dwFlags*/0,
                      utf16,
                      /*cchWideChar*/-1,
                      utf8,
                      required,
                      /*lpDefaultChar*/nullptr,
                      /*lpUsedDefaultChar*/nullptr);
  utf8[required] = 0;
  return utf8;
}

std::string build_optional_args(int argc, wchar_t *argv[]) {
  std::string s;
  for (int i = 0; i < argc; ++i) {
    s += to_utf8(argv[i]);
    s += '\0';
  }
  return s;
}

int wmain(int argc, wchar_t *argv[]) {
  if (argc < 3) {
    LOGINFO(L"\nUSAGE: pj.exe [-w] <PID> <FILE>[?ORDINAL] [ARGS]\n"
            L"  -w         Keep pj.exe running until the injected thread is signaled\n"
            L"  PID        PID of a target process to inject into\n"
            L"  FILE       DLL or a flat binary to inject\n"
            L"  ORDINAL    Orginal# of a function exported from the DLL to run in the target\n"
            L"             If not specified, FILE is assumed to be a flat binary.\n"
            L"  ARGS       Optional arguments that can be accessed as Package::args in the injected code\n"
            );
    return 1;
  }
  bool wait = _wcsicmp(argv[1], L"-W") == 0;
  int arg_index = wait ? 2 : 1;
  auto pair = split<wchar_t>(argv[arg_index + 1], L'?');
  auto optional_args = build_optional_args(argc - arg_index - 2,
                                           &argv[arg_index + 2]);
  Inject(_wtoi(argv[arg_index]),
         pair.first.c_str(),
         pair.second,
         optional_args,
         wait);
  return 0;
}
