#include <windows.h>
#include "event.h"

Event::Event(BOOL manualReset, BOOL true_to_set_signaled_initially, LPCWSTR name)
  : h_(CreateEvent(/*lpEventAttributes*/nullptr,
                   /*bManualReset*/manualReset,
                   /*bInitialState*/true_to_set_signaled_initially,
                   /*lpName*/name)),
    was_newly_created_(h_ && GetLastError() != ERROR_ALREADY_EXISTS)
{}

Event::~Event() {
  if (h_) {
    CloseHandle(h_);
  }
}

bool Event::WasNewlyCreated() const {
  return was_newly_created_;
}

BOOL Event::Signal() const {
  return SetEvent(h_);
}

BOOL Event::Reset() const {
  return ResetEvent(h_);
}

DWORD Event::Wait(DWORD timeout_in_ms) const {
  return WaitForSingleObject(h_, timeout_in_ms);
}
