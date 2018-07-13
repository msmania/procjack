class CodeTemplate {
  std::vector<uint8_t> blob_;

public:
  template<typename T>
  void Put(uint32_t offset, T value) {
    if (offset + sizeof(T) < blob_.size()) {
      uint8_t *p = blob_.data() + offset;
      *reinterpret_cast<T*>(p) = value;
    }
  }

  CodeTemplate(std::vector<uint8_t> &&blob);
  size_t Size() const;
  void CopyTo(uint8_t *destination) const;
};

class ExecutablePage final {
  uint32_t capacity_;
  uint8_t *base_;
  uint8_t *empty_head_;

public:
  ExecutablePage(uint32_t capacity);
  ~ExecutablePage();
  void *Push(const CodeTemplate &chunk);
};
