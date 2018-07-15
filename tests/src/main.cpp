#include <stdarg.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

void Log(const wchar_t *format, ...) {
  va_list v;
  va_start(v, format);
  vwprintf(format, v);
  va_end(v);
}
