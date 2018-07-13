#include <windows.h>
#include "event.h"

Event::Event()
  : h_(nullptr)
{}

Event::~Event() {
  if (h_) {
    CloseHandle(h_);
  }
}

bool Event::CreateIfNotCreatedYet(BOOL manualReset,
                                  BOOL true_to_set_signaled_initially,
                                  LPCWSTR name) {
  auto h_new = CreateEvent(/*lpEventAttributes*/nullptr,
                           /*bManualReset*/manualReset,
                           /*bInitialState*/true_to_set_signaled_initially,
                           /*lpName*/name);
  if (InterlockedCompareExchangePointer(&h_, h_new, nullptr)) {
    // h_ already has a value.  Keep using the current, discarding the new one.
    CloseHandle(h_new);
    return false;
  }

  // h_ is now h_new.  Replacement is done.
  return true;
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
