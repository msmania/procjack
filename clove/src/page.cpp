#include <windows.h>
#include <detours.h>
#include <vector>
#include <memory>
#include "page.h"

void Log(LPCWSTR format, ...);

bool CodePack::DetourAttachHelper(void *&detour_target,
                                  void *detour_destination) {
  PDETOUR_TRAMPOLINE trampoline;
  PVOID target, detour;
  LONG status = DetourAttachEx(&detour_target,
                               detour_destination,
                               &trampoline,
                               &target,
                               &detour);
  if (status == NO_ERROR) {
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

bool CodePack::DetourDetachHelper(void *&detour_target,
                                  void *detour_destination) {
  LONG status = DetourDetach(&detour_target, detour_destination);
  if (status != NO_ERROR) {
    Log(L"DetourDetach(%p, %p) failed with %08x\n",
        &detour_target,
        detour_destination,
        status);
  }
  return status == NO_ERROR;
}

bool CodePack::ActivateDetour(ExecutablePage &exec_page) {
  return DetourTransaction<&CodePack::ActivateDetourInternal>(exec_page);
}

bool CodePack::DeactivateDetour(ExecutablePage &exec_page) {
  return DetourTransaction<&CodePack::DeactivateDetourInternal>(exec_page);
}

ExecutablePage::ExecutablePage(uint32_t capacity)
  : capacity_(capacity),
    base_(reinterpret_cast<uint8_t*>(
            VirtualAlloc(/*lpAddress*/nullptr,
                         capacity,
                         MEM_COMMIT | MEM_RESERVE,
                         PAGE_EXECUTE_READWRITE))),
    empty_head_(base_)
{}

ExecutablePage::~ExecutablePage() {
  if (base_) {
    VirtualFree(base_, 0, MEM_RELEASE);
  }
}

void *ExecutablePage::Push(const CodePack &pack) {
  void *ret = nullptr;
  if (base_
      && empty_head_ + pack.Size() + 1 < base_ + capacity_) {
    ret = empty_head_;
    pack.CopyTo(empty_head_);
    empty_head_ += pack.Size();
    *empty_head_ = 0xCC;
    ++empty_head_;
  }
  else {
    Log(L"No enough space in ExecutablePage\n");
  }
  return ret;
}
