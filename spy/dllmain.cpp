#include <cstdint>
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

LRESULT HookProc(int code, WPARAM w, LPARAM l) {
  static decltype(&::CallNextHookEx) sCallNextHookEx = []() {
    Loader user32(L"user32.dll");
    return reinterpret_cast<decltype(&::CallNextHookEx)>(
        GetProcAddress(user32, "CallNextHookEx"));
  }();
  if (auto msg = reinterpret_cast<PCWPSTRUCT>(l)) {
    Log(L"HookProc: %08x %04x %08x %08x\n",
        msg->hwnd,
        msg->message,
        msg->wParam,
        msg->lParam);
  }
  return sCallNextHookEx(/*hhk*/nullptr, code, w, l);
}
