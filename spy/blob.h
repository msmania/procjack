class Blob {
private:
  HANDLE heap_;
  LPVOID buffer_;
  SIZE_T size_;

public:
  Blob();
  Blob(SIZE_T size);
  Blob(Blob &&other);
  ~Blob();

  operator PBYTE();
  operator LPCBYTE() const;
  template<typename T>
  T *As() {
    return reinterpret_cast<T*>(buffer_);
  }
  template<typename T>
  const T *As() const {
    return reinterpret_cast<const T*>(buffer_);
  }
  Blob &operator=(Blob &&other);
  SIZE_T Size() const;
  bool Alloc(SIZE_T size);
  void Release();
};
