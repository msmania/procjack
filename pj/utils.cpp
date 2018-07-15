#include <windows.h>
#include <string>
#include "utils.h"

std::string to_utf8(const wchar_t *utf16) {
  auto required = WideCharToMultiByte(CP_UTF8,
                                     /*dwFlags*/0,
                                     utf16,
                                     /*cchWideChar*/-1,
                                     /*lpMultiByteStr*/nullptr,
                                     0,
                                     /*lpDefaultChar*/nullptr,
                                     /*lpUsedDefaultChar*/nullptr);
  char *utf8 = new char[required + 1];
  WideCharToMultiByte(CP_UTF8,
                      /*dwFlags*/0,
                      utf16,
                      /*cchWideChar*/-1,
                      utf8,
                      required,
                      /*lpDefaultChar*/nullptr,
                      /*lpUsedDefaultChar*/nullptr);
  utf8[required] = 0;
  return utf8;
}

std::string build_optional_args(int argc, wchar_t *argv[]) {
  std::string s;
  for (int i = 0; i < argc; ++i) {
    s += to_utf8(argv[i]);
    s += '\0';
  }
  return s;
}
