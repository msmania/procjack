template<typename WideCharVersion>
struct xtoi {
  int operator()(const wchar_t *s) {
    return _wtoi(s);
  }
};

template<>
struct xtoi<std::false_type> {
  int operator()(const char *s) {
    return atoi(s);
  }
};

template<typename T>
std::pair<std::basic_string<T>, int> split(const std::basic_string<T> &s,
                                           T delim) {
  std::pair<std::basic_string<T>, int> ret;
  auto pos = s.find(delim);
  if (pos != std::basic_string<T>::npos) {
    ret.first = s.substr(0, pos);
    // xtoi should be faster than stoi
    ret.second = xtoi<std::is_same<wchar_t, T>::type>()(
      s.c_str() + pos + 1);
  }
  else {
    ret.first = s;
    ret.second = -1;
  }
  return ret;
}
