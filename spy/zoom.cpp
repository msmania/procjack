#include <cstdint>
#include <windows.h>
#include "../common.h"

constexpr UINT ZOOM_START_SHARING = 0x465;
constexpr UINT ZOOM_STOP_SHARING = 0x466;
constexpr UINT ZOOM_SHARE_MONITOR = 0x468;
constexpr UINT ZOOM_SHARE_WINDOW = 0x469;

void Log(LPCWSTR format, ...);

LRESULT ZoomWindowProc(int code, WPARAM wParam, LPARAM lParam) {
  if (auto msg = reinterpret_cast<PCWPSTRUCT>(lParam)) {
    switch (msg->message) {
    case ZOOM_START_SHARING:
      Log(L">> Zoom: Desktop sharing has started!\n");
      break;
    case ZOOM_STOP_SHARING:
      Log(L">> Zoom: Desktop sharing has stopped!\n");
      break;
    case ZOOM_SHARE_MONITOR:
      if (msg->lParam) {
        Log(L">> Zoom: source=%s\n",
            reinterpret_cast<LPCWSTR>(msg->lParam));
      }
      break;
    case ZOOM_SHARE_WINDOW:
      if (msg->lParam) {
        Log(L">> Zoom: source=HWND %p\n",
            *reinterpret_cast<HWND*>(msg->lParam));
      }
      break;
    }
  }
  return CallNextHookEx(/*hhk*/nullptr, code, wParam, lParam);
}
