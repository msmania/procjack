#include <windows.h>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../clove/src/page.h"
#include "../../clove/src/pack.h"

void Log(const wchar_t *format, ...);

template<int SIZE>
struct FixedSizePack : CodePack {
  uint8_t data_[SIZE];

  FixedSizePack(const uint8_t *data) {
    std::memcpy(data_, data, SIZE);
  }

  bool ActivateDetourInternal(ExecutablePages&) {
    return true;
  }

  bool DeactivateDetourInternal(ExecutablePages&) {
    return true;
  }

  size_t Size() const {
    return SIZE;
  }

  void Print() const {}

  void CopyTo(uint8_t *destination) const {
    std::memcpy(destination, data_, SIZE);
  }
};

void TestExecutablePagePerImageBase(ExecutablePages &exec_pages,
                                    HMODULE image_base,
                                    std::vector<const void*> &chunks) {
  constexpr uint64_t body = 0xc0ffee00beef1234;
  FixedSizePack<sizeof(body)> pack(reinterpret_cast<const uint8_t*>(&body));

  uint8_t *prev = nullptr;
  auto base = reinterpret_cast<uint8_t*>(image_base);
  for (int i = 0; i < 5; ++i) {
    auto chunk = reinterpret_cast<uint8_t*>(exec_pages.Push(pack, base));
    chunks.push_back(chunk);
    Log(L"%p\n", chunk);
    EXPECT_LT(abs(chunk - base), 0x7fffffff);
    EXPECT_EQ(*at<uint64_t>(chunk, 0), body);
    EXPECT_EQ(*at<uint8_t>(chunk, sizeof(body)), 0xCC);
    if (i > 0) {
      EXPECT_EQ(prev + sizeof(body) + 1, chunk);
    }
    prev = chunk;
  }
}

TEST(exec_pages, push) {
  std::vector<const void*> chunks;
  {
    ExecutablePages epages;
    TestExecutablePagePerImageBase(epages, GetModuleHandle(nullptr), chunks);
    TestExecutablePagePerImageBase(epages, GetModuleHandle(L"ntdll.dll"), chunks);
  }

  MEMORY_BASIC_INFORMATION meminfo;
  for (auto chunk : chunks) {
    auto p = reinterpret_cast<const void*>(
      reinterpret_cast<size_t>(chunk) & (-0x1000));
    ASSERT_EQ(VirtualQuery(p, &meminfo, sizeof(meminfo)), sizeof(meminfo));
    EXPECT_EQ(meminfo.State, static_cast<DWORD>(MEM_FREE));
  }
}

TEST(exec_pages, revert) {
  constexpr uint64_t body = 0xdeadbeef;
  FixedSizePack<sizeof(body)> pack(reinterpret_cast<const uint8_t*>(&body));

  std::vector<void*> chunks;
  ExecutablePages epages;
  for (int i = 0; i < 5; ++i) {
    auto chunk = epages.Push(pack, GetModuleHandle(nullptr));
    Log(L"%p\n", chunk);
    chunks.push_back(chunk);
    chunk = epages.Push(pack, GetModuleHandle(L"ntdll.dll"));
    Log(L"%p\n", chunk);
    chunks.push_back(chunk);
  }

  EXPECT_FALSE(epages.Revert(chunks[0]));

  for (auto it = chunks.rbegin(); it != chunks.rend(); ++it)
    EXPECT_TRUE(epages.Revert(*it));
}
