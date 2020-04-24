#include <cstdint>
#include <functional>
#include <windows.h>
#include <magnification.h>
#include <detours.h>
#include "../common.h"
#include "blob.h"

void Log(LPCWSTR format, ...);

class Loader final {
  HMODULE base_;

public:
  Loader(LPCWSTR name) : base_(LoadLibrary(name))
  {}

  ~Loader() {
    if (base_) FreeLibrary(base_);
  }

  operator bool() const { return !!base_; }
  operator HMODULE() const { return base_; }
};

namespace hook {
  void* orig_MagInitialize;
  BOOL WINAPI MagInitialize() {
    Log(L">> MagInitialize()\n");
    BOOL ret = reinterpret_cast<decltype(&::MagInitialize)>(
      orig_MagInitialize)();
    Log(L"<< MagInitialize: %d\n",
        ret);
    return ret;
  }

  void* orig_MagUninitialize;
  BOOL WINAPI MagUninitialize() {
    Log(L">> MagUninitialize()\n");
    BOOL ret = reinterpret_cast<decltype(&::MagUninitialize)>(
      orig_MagUninitialize)();
    Log(L"<< MagUninitialize: %d\n",
        ret);
    return ret;
  }

  void* orig_MagSetColorEffect;
  BOOL WINAPI MagSetColorEffect(HWND hwnd, PMAGCOLOREFFECT pEffect) {
    Log(L">> MagSetColorEffect(%p, [%f %f %f %f %f])\n",
        hwnd,
        pEffect->transform[0][0],
        pEffect->transform[0][1],
        pEffect->transform[0][2],
        pEffect->transform[0][3],
        pEffect->transform[0][4]);
    BOOL ret = reinterpret_cast<decltype(&::MagSetColorEffect)>(
      orig_MagSetColorEffect)(hwnd, pEffect);
    Log(L"<< MagSetColorEffect: %d\n",
        ret);
    return ret;
  }

  void* orig_MagSetWindowSource;
  BOOL WINAPI MagSetWindowSource(HWND hwnd, RECT rect) {
    static HWND h0 = nullptr;
    static RECT r0 = {};
    bool isSame = (h0 == hwnd && memcmp(&r0, &rect, sizeof(RECT)) == 0);
    if (!isSame) {
      h0 = hwnd, r0 = rect;
      Log(L">> MagSetWindowSource(%p, (%d, %d)-(%d, %d), %dx%d)\n",
          hwnd,
          rect.left, rect.top,
          rect.right, rect.bottom,
          rect.right - rect.left,
          rect.bottom - rect.top);
    }
    BOOL ret = reinterpret_cast<decltype(&::MagSetWindowSource)>(
      orig_MagSetWindowSource)(hwnd, rect);
    if (!isSame) {
      Log(L"<< MagSetWindowSource: %d\n",
          ret);
    }
    return ret;
  }

  void* orig_MagSetWindowFilterList;
  BOOL WINAPI MagSetWindowFilterList(HWND hwnd,
                                     DWORD dwFilterMode,
                                     int count,
                                     HWND *pHWND) {
    static HWND h0 = nullptr;
    static DWORD m0 = 0;
    static int c0 = 0;
    bool isSame = (h0 == hwnd
                   && m0 == dwFilterMode
                   && c0 == count);
    if (!isSame) {
      h0 = hwnd, m0 = dwFilterMode, c0 = count;
      Log(L">> MagSetWindowFilterList(%p, %d)\n",
          hwnd,
            dwFilterMode);
      for (int i = 0; i < count; ++i)
        Log(L"                          %p\n", pHWND[i]);
    }
    BOOL ret = reinterpret_cast<decltype(&::MagSetWindowFilterList)>(
      orig_MagSetWindowFilterList)(hwnd, dwFilterMode, count, pHWND);
    if (!isSame) {
      Log(L"<< MagSetWindowFilterList: %d\n",
          ret);
    }
    return ret;
  }
}

bool DetourTransaction(std::function<bool()> callback) {
  LONG status = DetourTransactionBegin();
  if (status != NO_ERROR) {
    Log(L"DetourTransactionBegin failed with %08x\n", status);
    return status;
  }

  if (callback()) {
    status = DetourTransactionCommit();
    if (status != NO_ERROR) {
      Log(L"DetourTransactionCommit failed with %08x\n", status);
    }
  }
  else {
    status = DetourTransactionAbort();
    if (status == NO_ERROR) {
      Log(L"Aborted transaction.\n");
    }
    else {
      Log(L"DetourTransactionAbort failed with %08x\n", status);
    }
  }
  return status == NO_ERROR;
}

bool DetourAttachHelper(void *&detour_target,
                        void *detour_destination) {
  PDETOUR_TRAMPOLINE trampoline;
  PVOID target, detour;
  LONG status = DetourAttachEx(&detour_target,
                               detour_destination,
                               &trampoline,
                               &target,
                               &detour);
  if (status == NO_ERROR) {
    // Detours updates the pointer at the moment of commit,
    // but we need a trampoline address right now.
    detour_target = trampoline;
    Log(L"Detouring: %p --> %p (trampoline:%p)\n",
        target,
        detour,
        trampoline);
  }
  else {
    Log(L"DetourAttach(%p, %p) failed with %08x\n",
        &detour_target,
        detour_destination,
        status);
  }
  return status == NO_ERROR;
}

#define DETOUR_ATTACH(func)\
  do {\
    hook::orig_##func = GetProcAddress(magDll, #func);\
    if (!DetourAttachHelper(hook::orig_##func, hook::##func))\
      return false;\
  } while (0)

void HookZoom(Package *package) {
  Loader magDll(L"magnification.dll");
  if (!magDll) return;

  DetourTransaction([&]() {
    //DETOUR_ATTACH(MagInitialize);
    //DETOUR_ATTACH(MagSetColorEffect);
    DETOUR_ATTACH(MagSetWindowSource);
    //DETOUR_ATTACH(MagSetWindowFilterList);
    //DETOUR_ATTACH(MagUninitialize);
    return true;
  });

  for (;;) {
    Sleep(100);
  }
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
  int cchLen = 2;
  Blob buf;
  for (;;) {
    if (!buf.Alloc(cchLen * sizeof(WCHAR))) {
      return FALSE;
    }

    int len = GetClassName(hwnd, buf.As<WCHAR>(), cchLen);
    if (!len) {
      Log(L"GetClassName failed - %08x\n", GetLastError());
      return TRUE;
    }

    if (len == cchLen - 1) {
      cchLen *= 2;
      continue;
    }

    if (wcscmp(buf.As<WCHAR>(), L"MozillaWindowClass") == 0) {
      RECT rect;
      if (!GetWindowRect(hwnd, &rect)) {
        Log(L"GetWindowRect failed - %08x\n", GetLastError());
        return TRUE;
      }
      if (rect.left == rect.right
          || rect.top == rect.bottom) {
        return TRUE;
      }

      if (!(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)) {
        return TRUE;
      }

      len = GetWindowTextLength(hwnd) + 1;
      Blob caption(len * sizeof(WCHAR));
      if (!GetWindowText(hwnd, caption.As<WCHAR>(), len)) {
        Log(L"GetWindowText failed - %08x\n", GetLastError());
        return TRUE;
      }

      Log(L"> %p: ""%s"" (%d, %d)-(%d, %d), %dx%d\n",
          hwnd,
          caption.As<WCHAR>(),
          rect.left, rect.top,
          rect.right, rect.bottom,
          rect.right - rect.left,
          rect.bottom - rect.top);
    }
    break;
  }
  return TRUE;
}

void SearchFox(Package *package) {
  if (!EnumWindows(EnumWindowsProc, 0)) {
    Log(L"EnumWindows failed - %08x\n", GetLastError());
  }
}
