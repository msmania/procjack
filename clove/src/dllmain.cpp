#include <windows.h>
#include <strsafe.h>

void Log(LPCWSTR format, ...) {
  WCHAR linebuf[1024];
  va_list v;
  va_start(v, format);
  StringCbVPrintf(linebuf, sizeof(linebuf), format, v);
  OutputDebugString(linebuf);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
  return TRUE;
}
