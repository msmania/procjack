#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../pj/utils.h"

uint64_t hex_to_uint64(const char *s);
std::vector<uint64_t> address_chain(char *str);

TEST(string, hex) {
  EXPECT_EQ(hex_to_uint64("a123ffff"), 0xa123ffff);
  EXPECT_EQ(hex_to_uint64("0xa123ffff"), 0xa123ffff);
  EXPECT_EQ(hex_to_uint64("0xa123ffff-"), 0);
  EXPECT_EQ(hex_to_uint64("7ffe`72a38000"), 0x7ffe72a38000ull);
  EXPECT_EQ(hex_to_uint64("0x7ffe`72a38000"), 0x7ffe72a38000ull);
  EXPECT_EQ(hex_to_uint64("0x7ffe?72a38000"), 0);

  EXPECT_EQ(hex_to_uint64("0xffffffff`fffffffe"), 0xfffffffffffffffeull);
  EXPECT_EQ(hex_to_uint64("0xFFFFFFFFFFFFFFFD"), 0xfffffffffffffffdull);
  EXPECT_EQ(hex_to_uint64("ffffffff`fffffffc"), 0xfffffffffffffffcull);
  EXPECT_EQ(hex_to_uint64("FFFFFFFFFFFFFFFB"), 0xfffffffffffffffbull);
  EXPECT_EQ(hex_to_uint64("0xfffffffa"), 0xfffffffa);
  EXPECT_EQ(hex_to_uint64("FFFFFFF9"), 0xfffffff9);

  EXPECT_EQ(hex_to_uint64("0ffffffffffffffff"), 0xffffffffffffffffull);
  EXPECT_EQ(hex_to_uint64("0x0000000000000000ffffffffffffffff"), 0xffffffffffffffffull);
  EXPECT_EQ(hex_to_uint64("1ffffffffffffffff"), 0);
}

TEST(string, address_chain) {
  auto checker = [](const char *input, std::vector<uint64_t> expected) {
    char mutable_string[64];
    strcpy(mutable_string, input);
    EXPECT_EQ(address_chain(mutable_string), expected);
  };
  checker("1-2", {1, 2});
  checker("3-2-1", {3, 2, 1});
  checker("5f40747f-5f407493-5f4074c1-5f4074ca",
          {0x5f40747f, 0x5f407493, 0x5f4074c1, 0x5f4074ca});
  checker("00007ff6`965a3fba-00007ff6`965a3ffd-00007ff6`965a4024",
          {0x00007ff6965a3fba, 0x00007ff6965a3ffd, 0x00007ff6965a4024});
  checker("", {0});
  checker("-", {0, 0});
  checker("--", {0, 0, 0});
  checker("42-", {0x42, 0});
  checker("-43", {0, 0x43});
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
