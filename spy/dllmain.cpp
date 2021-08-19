#include <cstdint>
#include <memory>
#include <vector>
#include <windows.h>
#include <strsafe.h>
#include "../common.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL,
                    DWORD fdwReason,
                    LPVOID lpvReserved) {
  return TRUE;
}

void Log(LPCWSTR format, ...) {
  WCHAR linebuf[1024];
  va_list v;
  va_start(v, format);
  StringCbVPrintf(linebuf, sizeof(linebuf), format, v);
  va_end(v);
  OutputDebugString(linebuf);
}

void Hello(Package *package) {
  Log(L"Injection Package:\n"
      L" Page = %p\n"
      L" DLL  = %s\n"
      L" PEB  = %p\n"
      L" Args = %hs\n",
      package,
      reinterpret_cast<LPCWSTR>(package->nw.dllpath),
      package->peb.p,
      package->nw.args);
}

uint64_t hex_to_uint64(const char *s) {
  auto htoc = [](char c) -> int {
    return
      (c >= '0' && c <= '9') ? c - '0'
      : (c >= 'a' && c <= 'f') ? c - 'a' + 10
      :  (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
  };
  uint64_t ret = 0;
  const char *p = s;
  uint32_t valid_chars = 0;
  for (; *p && valid_chars <= 16; ++p) {
    if (p == s + 1
        && s[1] == 'x'
        && s[0] == '0') {
      valid_chars = 0;
      ret = 0;
    }
    else if (*p != '`') {
      int c = htoc(*p);
      if (c < 0) return 0;
      ret = (ret << 4) | c;
      if (ret > 0) ++valid_chars;
    }
  }
  return valid_chars <= 16 ? ret : 0;
}

void Touch(void* aAddr, size_t aSize) {
  volatile uint8_t x = 0;
  auto base = reinterpret_cast<uint8_t*>(aAddr);
  for (int64_t i = 0, pages = aSize >> 12; i < pages; ++i) {
    auto pageBegin = base + (i << 12);
    int offset = rand() % (1 << 12);
    x ^= *(pageBegin + offset);
  }
}

struct PageFreer {
  void operator()(void* aAddr) {
    ::VirtualFree(aAddr, 0, MEM_RELEASE);
  }
};
struct HandleCloser {
  void operator()(void* aHandle) {
    ::CloseHandle(aHandle);
  }
};
using PageT = std::unique_ptr<void, PageFreer>;
using HandleT = std::unique_ptr<void, HandleCloser>;

void EatMemory(Package *package) {
  const wchar_t* kEventName = L"KeepMeSpyLoaded";
  uint64_t goal = hex_to_uint64(package->nw.args);

  if (!goal) {
    HandleT wait(::OpenEventW(
        EVENT_MODIFY_STATE,
        /*bInheritHandle*/FALSE,
        kEventName));
    if (!wait.get()) {
      Log(L"OpenEventW failed - %08lx\n", ::GetLastError());
      return;
    }

    if (!::SetEvent(wait.get())) {
      Log(L"SetEvent failed - %08lx\n", ::GetLastError());
    }
    return;
  }

  constexpr size_t kMinGranularity = 1 << 20; // 1MB
  size_t currentSize = goal;
  size_t consumed = 0;
  std::vector<PageT> stock;
  while (goal >= kMinGranularity) {
    PageT p(::VirtualAlloc(nullptr, currentSize,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!p) {
      DWORD gle = ::GetLastError();
      if (gle != ERROR_COMMITMENT_LIMIT) {
        Log(L"VirtualAlloc failed - %08lx\n", gle);
        break;
      }

      if (currentSize <= kMinGranularity) {
        Log(L"No more commit space.\n");
        break;
      }

      currentSize >>= 1;
      continue;
    }
    Log(L"VirtualAlloc(0x%llx) -> %p\n", currentSize, p.get());
    goal -= currentSize;
    consumed += currentSize;
    Touch(p.get(), currentSize);
    stock.emplace_back(std::move(p));
  }

  Log(L"Ate 0x%llx bytes (%d MB)\n", consumed, consumed >> 20);

  HandleT wait(::CreateEventW(
      /*lpEventAttributes*/nullptr,
      /*bManualReset*/TRUE,
      /*bInitialState*/FALSE,
      kEventName));
  if (!wait.get()) {
    Log(L"CreateEventW failed - %08lx\n", ::GetLastError());
    return;
  }

  ::WaitForSingleObject(wait.get(), INFINITE);
  stock = std::vector<PageT>();
  Log(L"Freed 0x%llx bytes (%d MB)\n", consumed, consumed >> 20);
}
