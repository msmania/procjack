//
// dllmain.cpp
//

#include <windows.h>

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL,
                    _In_ DWORD fdwReason,
                    _In_ LPVOID lpvReserved) {
    return TRUE;
}

int Hello() {
    OutputDebugString(L"Hello from spy.dll! :)\n");
    return 42;
}
