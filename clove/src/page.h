class ExecutablePage;

class CodePack {
  template<bool (CodePack::*F)(ExecutablePage&)>
  bool DetourTransaction(ExecutablePage &exec_page) {
    LONG status = DetourTransactionBegin();
    if (status != NO_ERROR) {
      Log(L"DetourTransactionBegin failed with %08x\n", status);
      return status;
    }

    if ((this->*F)(exec_page)) {
      status = DetourTransactionCommit();
      if (status != NO_ERROR) {
        Log(L"DetourTransactionCommit failed with %08x\n", status);
      }
    }
    else {
      status = DetourTransactionAbort();
      if (status == NO_ERROR) {
        Log(L"Aborted transaction.\n");
      }
      else {
        Log(L"DetourTransactionAbort failed with %08x\n", status);
      }
    }
    return status == NO_ERROR;
  }

  virtual bool ActivateDetourInternal(ExecutablePage &exec_page) = 0;
  virtual bool DeactivateDetourInternal(ExecutablePage &exec_page) = 0;

protected:
  static bool DetourAttachHelper(void *&detour_target,
                                 void *detour_destination);
  static bool DetourDetachHelper(void *&detour_target,
                                 void *detour_destination);

public:
  virtual size_t Size() const = 0;
  virtual void Print() const = 0;
  virtual void CopyTo(uint8_t *destination) const = 0;
  bool ActivateDetour(ExecutablePage &exec_page);
  bool DeactivateDetour(ExecutablePage &exec_page);
};

class ExecutablePage final {
  uint32_t capacity_;
  uint8_t *base_;
  uint8_t *empty_head_;

public:
  ExecutablePage(uint32_t capacity);
  ~ExecutablePage();
  void *Push(const CodePack &pack);
};
