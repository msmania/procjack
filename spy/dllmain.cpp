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
