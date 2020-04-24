#include <windows.h>
#include "blob.h"

void Log(LPCWSTR Format, ...);

void Blob::Release() {
  if (buffer_) {
    HeapFree(heap_, 0, buffer_);
    buffer_ = nullptr;
    size_ = 0;
  }
}

Blob::Blob()
  : heap_(GetProcessHeap()),
    buffer_(nullptr),
    size_(0)
{}

Blob::Blob(SIZE_T size)
  : heap_(GetProcessHeap()),
    buffer_(nullptr),
    size_(0)
{
  Alloc(size);
}

Blob::Blob(Blob &&other)
  : heap_(other.heap_),
    buffer_(other.buffer_),
    size_(other.size_) {
  other.heap_ = nullptr;
  other.buffer_ = nullptr;
  other.size_ = 0;
}

Blob::~Blob() {
  Release();
}

Blob::operator PBYTE() {
  return reinterpret_cast<PBYTE>(buffer_);
}

Blob::operator LPCBYTE() const {
  return reinterpret_cast<LPCBYTE>(buffer_);
}

Blob &Blob::operator=(Blob &&other) {
  if (this != &other) {
    Release();
    heap_ = other.heap_;
    buffer_ = other.buffer_;
    size_ = other.size_;
    other.buffer_ = nullptr;
    other.size_ = 0;
  }
  return *this;
}

SIZE_T Blob::Size() const {
  return size_;
}

bool Blob::Alloc(SIZE_T size) {
  if (buffer_) {
    buffer_ = HeapReAlloc(GetProcessHeap(), 0, buffer_, size);
    if (buffer_) {
      size_ = size;
    }
    else {
      Log(L"HeapReAlloc failed - %08x\n", GetLastError());
    }
  }
  else if (size > 0) {
    buffer_ = HeapAlloc(GetProcessHeap(), 0, size);
    if (buffer_) {
      size_ = size;
    }
    else {
      Log(L"HeapAlloc failed - %08x\n", GetLastError());
    }
  }
  return buffer_ != nullptr;
}
