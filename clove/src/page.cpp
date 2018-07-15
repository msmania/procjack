#include <windows.h>
#include <memory>
#include <detours.h>
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);

ExecutablePages::ExecutablePage::ExecutablePage(uint32_t capacity, void *buffer)
  : next_(nullptr),
    capacity_(capacity),
    base_(reinterpret_cast<uint8_t*>(buffer)),
    empty_head_(base_)
{}

ExecutablePages::ExecutablePage::~ExecutablePage() {
  if (base_)
    VirtualFree(base_, 0, MEM_RELEASE);

  for (ExecutablePage *p = next_; p;) {
    auto next = p->next_;
    p->next_ = nullptr;
    delete p;
    p = next;
  }
}

ExecutablePages::Slot::Slot(Slot *next, uint8_t *start)
  : next_(next), start_(start)
{}

ExecutablePages::Slot::~Slot() {
  for (Slot *p = next_; p;) {
    auto next = p->next_;
    p->next_ = nullptr;
    delete p;
    p = next;
  }
}

void *ExecutablePages::ExecutablePage::Push(const CodePack &pack) {
  uint8_t *previous_head = nullptr;
  if (base_
      && empty_head_ + pack.Size() + 1 < base_ + capacity_) {
    previous_head = empty_head_;
    pack.CopyTo(empty_head_);
    empty_head_ += pack.Size();
    *empty_head_ = 0xCC;
    ++empty_head_;
    active_slots_ = std::make_unique<Slot>(active_slots_.release(),
                                           previous_head);
  }
  else {
    Log(L"No enough space in ExecutablePage\n");
  }
  return previous_head;
}

const void *ExecutablePages::ExecutablePage::TryPush(const CodePack &pack) const {
  return (base_
          && empty_head_ + pack.Size() + 1 < base_ + capacity_)
         ? empty_head_
         : nullptr;
}

bool ExecutablePages::ExecutablePage::Revert(const void *last_pushed) {
  if (active_slots_
      && active_slots_->start_ == last_pushed) {
    empty_head_ = active_slots_->start_;
    auto dismissed = std::move(active_slots_);
    active_slots_.reset(std::move(dismissed->next_));
    return true;
  }
  return false;
}

bool IsJumpable(const void *from, const void *to) {
  int64_t a = reinterpret_cast<int64_t>(from);
  int64_t b = reinterpret_cast<int64_t>(to);
  return abs(a - b) <= 0x7fffffff;
}

void *ExecutablePages::Push(const CodePack &pack, const void *source) {
  for (auto page = active_head_.get(); page; page = page->next_) {
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
        new_page->next_ = active_head_.release();
        active_head_ = std::move(new_page);
        return active_head_->Push(pack);
      }
    }
  }
  Log(L"Failed to allocate a region near %p\n", source);
  return nullptr;
}

bool ExecutablePages::Revert(const void *last_pushed) {
  for (auto page = active_head_.get(); page; page = page->next_) {
    if (page->Revert(last_pushed))
      return true;
  }
  return false;
}
