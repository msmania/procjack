#include <windows.h>
#include <vector>
#include <memory>
#include "page.h"

CodeTemplate::CodeTemplate(std::vector<uint8_t> &&blob)
  : blob_(blob)
{}

size_t CodeTemplate::Size() const {
  return blob_.size();
}

void CodeTemplate::CopyTo(uint8_t *destination) const {
  std::memcpy(destination, blob_.data(), blob_.size());
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

void *ExecutablePage::Push(const CodeTemplate &chunk) {
  void *ret = nullptr;
  if (base_
      && empty_head_ + chunk.Size() + 1 < base_ + capacity_) {
    ret = empty_head_;
    chunk.CopyTo(empty_head_);
    empty_head_ += chunk.Size();
    *empty_head_ = 0xCC;
    ++empty_head_;
  }
  return ret;
}
