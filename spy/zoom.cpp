#include <functional>
#include <windows.h>
#include <magnification.h>
#include <detours.h>
#include "../common.h"
#include "blob.h"

//#define ZOOM_LOG 1

void Log(LPCWSTR format, ...);

class SearchByWindowClassName final {
  static BOOL CALLBACK EnumWindowsProc(HWND window, LPARAM lParam) {
    if (!lParam) return FALSE;
    return
      reinterpret_cast<SearchByWindowClassName*>(lParam)->EnumInternal(window);
  }

  static Blob GetClassNameSafely(HWND window) {
    int cchLen = 64;
    Blob blob;
    for (;;) {
      if (!blob.Alloc(cchLen * sizeof(WCHAR))) {
        break;
      }

      int len = GetClassName(window, blob.As<WCHAR>(), cchLen);
      if (!len) {
        Log(L"GetClassName failed - %08x\n", GetLastError());
        break;
      }

      if (len == cchLen - 1) {
        cchLen *= 2;
        continue;
      }

      return blob;
    }

    blob.Release();
    return blob;
  }

  using CallbackFunc = std::function<void(HWND, const RECT&)>;

  LPCWSTR targetName_;
  CallbackFunc callback_;

  BOOL EnumInternal(HWND window) {
    Blob className = GetClassNameSafely(window);
    if (targetName_
        && (className.Size() == 0
            || wcscmp(className.As<WCHAR>(), targetName_) != 0)) {
      return TRUE;
    }

    RECT rect;
    if (!GetWindowRect(window, &rect)) {
      Log(L"GetWindowRect failed - %08x\n", GetLastError());
      return TRUE;
    }
    if (IsRectEmpty(&rect)) {
      return TRUE;
    }

    LONG styles = GetWindowLong(window, GWL_STYLE);
    if (!(styles & WS_VISIBLE)
        || (styles & WS_MINIMIZE)) {
      return TRUE;
    }

    if (callback_) {
      callback_(window, rect);
    }

    return TRUE;
  }

public:
  SearchByWindowClassName()
    : targetName_(nullptr)
  {}

  void Sync(LPCWSTR className, CallbackFunc callback) {
    targetName_ = className;
    callback_ = callback;
    if (!EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this))) {
      Log(L"EnumWindows failed - %08x\n", GetLastError());
    }
  }
};

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
    Log(L">> MagSetColorEffect(%p,\n"
         "                     [%f %f %f %f %f]\n"
         "                     [%f %f %f %f %f]\n"
         "                     [%f %f %f %f %f]\n"
         "                     [%f %f %f %f %f]\n"
         "                     [%f %f %f %f %f])\n",
        hwnd,
        pEffect->transform[0][0],
        pEffect->transform[0][1],
        pEffect->transform[0][2],
        pEffect->transform[0][3],
        pEffect->transform[0][4],
        pEffect->transform[1][0],
        pEffect->transform[1][1],
        pEffect->transform[1][2],
        pEffect->transform[1][3],
        pEffect->transform[1][4],
        pEffect->transform[2][0],
        pEffect->transform[2][1],
        pEffect->transform[2][2],
        pEffect->transform[2][3],
        pEffect->transform[2][4],
        pEffect->transform[3][0],
        pEffect->transform[3][1],
        pEffect->transform[3][2],
        pEffect->transform[3][3],
        pEffect->transform[3][4],
        pEffect->transform[4][0],
        pEffect->transform[4][1],
        pEffect->transform[4][2],
        pEffect->transform[4][3],
        pEffect->transform[4][4]);
    BOOL ret = reinterpret_cast<decltype(&::MagSetColorEffect)>(
      orig_MagSetColorEffect)(hwnd, pEffect);
    Log(L"<< MagSetColorEffect: %d\n",
        ret);
    return ret;
  }

  void* orig_MagSetWindowSource;
  BOOL WINAPI MagSetWindowSource(HWND hwnd, RECT rectShared) {
#if ZOOM_LOG
    static HWND h0 = nullptr;
    static RECT r0 = {};
    bool isSame = (h0 == hwnd && memcmp(&r0, &rectShared, sizeof(RECT)) == 0);
    if (!isSame) {
      h0 = hwnd, r0 = rectShared;
      Log(L">> MagSetWindowSource(%p, (%d, %d)-(%d, %d), %dx%d)\n",
          hwnd,
          rectShared.left, rectShared.top,
          rectShared.right, rectShared.bottom,
          rectShared.right - rectShared.left,
          rectShared.bottom - rectShared.top);
    }
#else
    SearchByWindowClassName searcher;
    searcher.Sync(L"MozillaWindowClass",
                  [&rectShared]
                  (HWND window, const RECT &rectFirefox) {
                    SYSTEMTIME st;
                    GetSystemTime(&st);

                    RECT intersect;
                    if (IntersectRect(&intersect, &rectShared, &rectFirefox)) {
                      Log(L"%02d:%02d:%02d.%d: Firefox window %p is shared!!\n",
                          st.wHour,
                          st.wMinute,
                          st.wSecond,
                          st.wMilliseconds,
                          window);
                    }
                  });
#endif

    BOOL ret = reinterpret_cast<decltype(&::MagSetWindowSource)>(
      orig_MagSetWindowSource)(hwnd, rectShared);

#if ZOOM_LOG
    if (!isSame) {
      Log(L"<< MagSetWindowSource: %d\n",
          ret);
    }
#endif

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

  void* orig_SendMessageW;
  LRESULT WINAPI SendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    constexpr UINT ZOOM_START_SHARING = 0x465;
    constexpr UINT ZOOM_STOP_SHARING = 0x466;
    constexpr UINT ZOOM_SHARE_MONITOR = 0x468;
    constexpr UINT ZOOM_SHARE_WINDOW = 0x469;

    switch (Msg) {
    case ZOOM_START_SHARING:
      Log(L">> Zoom: Desktop sharing has started!\n");
      break;
    case ZOOM_STOP_SHARING:
      Log(L">> Zoom: Desktop sharing has stopped!\n");
      break;
    case ZOOM_SHARE_MONITOR:
      if (lParam) {
        Log(L">> Zoom: source=%s\n", reinterpret_cast<LPCWSTR>(lParam));
      }
      break;
    case ZOOM_SHARE_WINDOW:
      if (lParam) {
        Log(L">> Zoom: source=HWND %p\n", *reinterpret_cast<HWND*>(lParam));
      }
      break;
    }

    BOOL ret = reinterpret_cast<decltype(&::SendMessageW)>(
      orig_SendMessageW)(hWnd, Msg, wParam, lParam);
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

void SearchFox(Package *package) {
  SearchByWindowClassName searcher;
  searcher.Sync(L"MozillaWindowClass", [](HWND window, const RECT &rect) {
    int len = GetWindowTextLength(window) + 1;
    Blob caption(len * sizeof(WCHAR));
    if (!GetWindowText(window, caption.As<WCHAR>(), len)) {
      Log(L"GetWindowText failed - %08x\n", GetLastError());
      return;
    }

    Log(L"> %p: ""%s"" (%d, %d)-(%d, %d), %dx%d\n",
        window,
        caption.As<WCHAR>(),
        rect.left, rect.top,
        rect.right, rect.bottom,
        rect.right - rect.left,
        rect.bottom - rect.top);
  });
}

void HookZoom2(Package *package) {
  Loader magDll(L"user32.dll");
  if (!magDll) return;

  DetourTransaction([&]() {
    DETOUR_ATTACH(SendMessageW);
    return true;
  });

  for (;;) {
    Sleep(100);
  }
}
