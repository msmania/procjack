#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL,
                    DWORD fdwReason,
                    LPVOID lpvReserved) {
  return TRUE;
}

int Hello() {
  OutputDebugString(L"Hello from spy.dll! :)\n");
  return 42;
}
