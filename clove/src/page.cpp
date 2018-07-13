#include <windows.h>
#include <detours.h>
#include <vector>
#include <memory>
#include "page.h"

void Log(LPCWSTR format, ...);

CodePack::~CodePack()
{}

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

bool CodePack::ActivateDetour(ExecutablePages &exec_pages) {
  return DetourTransaction<&CodePack::ActivateDetourInternal>(exec_pages);
}

bool CodePack::DeactivateDetour(ExecutablePages &exec_pages) {
  return DetourTransaction<&CodePack::DeactivateDetourInternal>(exec_pages);
}

struct ExecutablePage final {
  ExecutablePage *next_;
  uint32_t capacity_;
  uint8_t *base_;
  uint8_t *empty_head_;

  ExecutablePage(uint32_t capacity, void *buffer)
    : next_(nullptr),
      capacity_(capacity),
      base_(reinterpret_cast<uint8_t*>(buffer)),
      empty_head_(base_)
  {}

  ~ExecutablePage() {
    if (base_) {
      VirtualFree(base_, 0, MEM_RELEASE);
    }
  }

  void *Push(const CodePack &pack) {
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

  const void *TryPush(const CodePack &pack) const {
    return (base_
            && empty_head_ + pack.Size() + 1 < base_ + capacity_)
           ? empty_head_
           : nullptr;
  }
};

bool IsJumpable(const void *from, const void *to) {
  int64_t a = reinterpret_cast<int64_t>(from);
  int64_t b = reinterpret_cast<int64_t>(to);
  return abs(a - b) < 0x7fffffff;
}

void *ExecutablePages::Push(const CodePack &pack, const void *source) {
  ExecutablePage *page;
  for (page = active_head_; page; page = page->next_) {
    auto candidate = page->TryPush(pack);
    if (candidate && IsJumpable(candidate, source)) {
      return page->Push(pack);
    }
  }

  DWORD new_region_size = 0;
  if (auto new_region_address =
             DetourAllocateRegionJumpableFromAddress(source,
                                                     &new_region_size)) {
    if (auto new_page = std::make_unique<ExecutablePage>(new_region_size,
                                                         new_region_address)) {
      auto candidate = new_page->TryPush(pack);
      if (candidate && IsJumpable(candidate, source)) {
        new_page->next_ = active_head_;
        active_head_ = new_page.release();
        return active_head_->Push(pack);
      }
    }
  }
  Log(L"Failed to allocate a region near %p\n", source);
  return nullptr;
}
