#include <cstdint>
#include <utility>

uint64_t htos(const char *s) {
  auto htoc = [](char c) -> int {
    return
      (c >= '0' && c <= '9') ? c - '0'
      : (c >= 'a' && c <= 'f') ? c - 'a' + 10
      :  (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
  };
  uint64_t ret = 0;
  const char *p = s;
  uint32_t valid_chars = 0;
  for (; *p && valid_chars <= 16; ++p) {
    if (p == s + 1
        && s[1] == 'x'
        && s[0] == '0') {
      valid_chars = 0;
      ret = 0;
    }
    else if (*p != '`') {
      int c = htoc(*p);
      if (c < 0) return 0;
      ret = (ret << 4) | c;
      if (ret > 0) ++valid_chars;
    }
  }
  return valid_chars <= 16 ? ret : 0;
}

std::pair<uint64_t, uint64_t> address_range(char *str) {
  std::pair<uint64_t, uint64_t> ret;
  auto hyphen = strchr(str, '-');
  if (hyphen) {
    *hyphen = '\0';
    ret.first = htos(str);
    ret.second = htos(hyphen + 1);
    if (ret.first > ret.second)
      std::swap(ret.first, ret.second);
  }
  return ret;
}

#ifdef GTEST
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
#endif
