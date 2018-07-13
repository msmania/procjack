class PackageCreator final {
  uint32_t blob_length_;
  union {
    void *blob_;
    Package *package_;
  };

  void FillShellCode(bool is_64bit, uint16_t ordinal);

public:
  template<typename T> const T *As() const {
    return reinterpret_cast<const T*>(blob_);
  }

  PackageCreator();
  ~PackageCreator();

  const uint32_t Size() const;
  bool Fill(bool is_64bit,
            LPCWSTR filepath,
            int32_t ordinal,
            const std::string &args);
};
