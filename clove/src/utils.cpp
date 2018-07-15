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
