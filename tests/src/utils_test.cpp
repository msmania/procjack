#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../pj/utils.h"

uint64_t htos(const char *s);
std::pair<uint64_t, uint64_t> address_range(char *str);

TEST(string, hex) {
  EXPECT_EQ(htos("a123ffff"), 0xa123ffff);
  EXPECT_EQ(htos("0xa123ffff"), 0xa123ffff);
  EXPECT_EQ(htos("0xa123ffff-"), 0);
  EXPECT_EQ(htos("7ffe`72a38000"), 0x7ffe72a38000ull);
  EXPECT_EQ(htos("0x7ffe`72a38000"), 0x7ffe72a38000ull);
  EXPECT_EQ(htos("0x7ffe?72a38000"), 0);

  EXPECT_EQ(htos("0xffffffff`fffffffe"), 0xfffffffffffffffeull);
  EXPECT_EQ(htos("0xFFFFFFFFFFFFFFFD"), 0xfffffffffffffffdull);
  EXPECT_EQ(htos("ffffffff`fffffffc"), 0xfffffffffffffffcull);
  EXPECT_EQ(htos("FFFFFFFFFFFFFFFB"), 0xfffffffffffffffbull);
  EXPECT_EQ(htos("0xfffffffa"), 0xfffffffa);
  EXPECT_EQ(htos("FFFFFFF9"), 0xfffffff9);

  EXPECT_EQ(htos("0ffffffffffffffff"), 0xffffffffffffffffull);
  EXPECT_EQ(htos("0x0000000000000000ffffffffffffffff"), 0xffffffffffffffffull);
  EXPECT_EQ(htos("1ffffffffffffffff"), 0);
}

TEST(string, address_range) {
  auto checker = [](const char *input, std::pair<uint64_t, uint64_t> expected) {
    char mutable_string[64];
    strcpy(mutable_string, input);
    EXPECT_EQ(address_range(mutable_string), expected);
  };
  checker("1-2", {1, 2});
  checker("00007ff7`407f4096-00007ff7`407f40e8", {0x00007ff7407f4096, 0x00007ff7407f40e8});
  checker("0xFFFFFFFFFFFFFFFF-7ffe72a38000", {0x7ffe72a38000, 0xFFFFFFFFFFFFFFFF});
  checker("00007ff7`407f4096-", {0, 0x00007ff7407f4096});
  checker("-00007ff7`407f40e8", {0, 0x00007ff7407f40e8});
  checker("-", {0, 0});
  checker("", {0, 0});
}

template<typename T>
void pair_checker(const T *input,
                  const T *expected_first,
                  int expected_second) {
  auto pair = split<T>(input, static_cast<T>('?'));
  EXPECT_EQ(pair.first, expected_first);
  EXPECT_EQ(pair.second, expected_second);
};

TEST(string, split) {
  pair_checker<wchar_t>(L"W:\\01.dll?101", L"W:\\01.dll", 101);
  pair_checker<char>("D:\\01.dll?101", "D:\\01.dll", 101);
  pair_checker<char>("D:\\02.dll?102?9", "D:\\02.dll", 102);
  pair_checker<char>("D:\\03.dll?103q9", "D:\\03.dll", 103);
  pair_checker<char>("D:\\04.dll??", "D:\\04.dll", 0);
  pair_checker<char>("D:\\05.dll?abc", "D:\\05.dll", 0);
  pair_checker<char>("D:\\06.dll", "D:\\06.dll", -1);
}
