class PackageCreator final {
  union {
    void *blob_;
    Package *package_;
  };

  void FillShellCode(bool is_64bit, uint16_t ordinal);

public:
  PackageCreator();
  ~PackageCreator();
  bool Fill(bool is_64bit,
            LPCWSTR filepath,
            int32_t ordinal,
            const std::string &args);
  PVOID Inject(HANDLE target_process);
};
