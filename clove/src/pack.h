template<typename T>
static T *at(void *base, int32_t offset) {
  return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(base) + offset);
}

template<typename T>
static const T *at(const void *base, int32_t offset) {
  return reinterpret_cast<const T*>(
    reinterpret_cast<const uint8_t*>(base) + offset);
}

class ExecutablePages;

class CodePack {
  template<bool (CodePack::*F)(ExecutablePages&)>
  bool DetourTransaction(ExecutablePages &exec_pages) {
    LONG status = DetourTransactionBegin();
    if (status != NO_ERROR) {
      Log(L"DetourTransactionBegin failed with %08x\n", status);
      return status;
    }

    if ((this->*F)(exec_pages)) {
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

  virtual bool ActivateDetourInternal(ExecutablePages &exec_pages) = 0;
  virtual bool DeactivateDetourInternal(ExecutablePages &exec_pages) = 0;

protected:
  static bool DetourAttachHelper(void *&detour_target,
                                 void *detour_destination);
  static bool DetourDetachHelper(void *&detour_target,
                                 void *detour_destination);
  static const void *PutImmediateNearJump(void *jump_from,
                                          const void *jump_to);

public:
  virtual ~CodePack();
  virtual size_t Size() const = 0;
  virtual void Print() const = 0;
  virtual void CopyTo(uint8_t *destination) const = 0;
  bool ActivateDetour(ExecutablePages &exec_pages);
  bool DeactivateDetour(ExecutablePages &exec_pages);
};
