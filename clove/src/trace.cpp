#include <windows.h>
#include <vector>
#include <memory>
#include <intrin.h>
#include "../../common.h"
#include "event.h"
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);
uint64_t hex_to_uint64(const char *s);
void WaitAndThenCleanup();

extern "C" {
  extern uint8_t FunctionTracePack_Start;
  extern uint8_t FunctionTracePack_End;
}

extern ExecutablePages g_exec_pages;
extern std::vector<std::unique_ptr<CodePack>> g_packs;
extern SRWLOCK g_shim_lock;

struct FunctionTracePack final : public CodePack {
  __declspec(noinline)
  void Push(uint64_t ticks,
            void **stack,
            void *arg1,
            void *arg2,
            void *arg3,
            void*,
            void*,
            void*,
            void*,
            void*) {
#if 0
    Log(L"[%d] %llu: %S %p %p %p %p...\n",
        call_count_,
        ticks,
        __FUNCTION__,
        stack,
        arg1,
        arg2,
        arg3);
#endif
    InterlockedIncrement(&call_count_);
    AcquireSRWLockExclusive(&record_lock_);
    if (records_.size() < max_records_)
      records_.emplace_back(ticks, *stack, arg1, arg2, arg3);
    ReleaseSRWLockExclusive(&record_lock_);
  }

  class FunctionTraceTemplate : public CodeTemplate {
#ifdef _WIN64
    static constexpr uint32_t offset_Trampoline = 0x80;
    static constexpr uint32_t offset_FunctionTracePack = 0xd7;
    static constexpr uint32_t offset_FunctionTracePackPush = 0xe1;
#else
    static constexpr uint32_t offset_Trampoline = 0x43;
    static constexpr uint32_t offset_ReturnUnwind = 0x52;
    static constexpr uint32_t offset_FunctionTracePack = 0x9d;
    static constexpr uint32_t offset_FunctionTracePackPush = 0xa2;
#endif

  public:
    size_t Size() const {
      return &FunctionTracePack_End - &FunctionTracePack_Start;
    }

    void CopyTo(void *destination) const {
      std::memcpy(destination, &FunctionTracePack_Start, Size());
    }

    bool Fill(void *start_address,
              const FunctionTracePack *pack,
              decltype(&FunctionTracePack::Push) func_to_push,
              const void *trampoline) const {
      *at<const void*>(start_address, offset_Trampoline) = trampoline;
      *at<const void*>(start_address, offset_FunctionTracePack) = pack;
      *at<decltype(func_to_push)>(start_address, offset_FunctionTracePackPush)
        = func_to_push;
#ifdef _X86_
      *at<const void*>(start_address, offset_ReturnUnwind)
        = at<const void>(start_address, Size() - 2);
#endif
      return true;
    }
  };

  struct Record {
    const uint32_t tid_;
    const uint64_t ticks_;
    const void *ret_;
    const void *args_[3];
    Record(uint64_t ticks,
           void *ret,
           void *arg1,
           void *arg2,
           void *arg3)
      : tid_(GetCurrentThreadId()),
        ticks_(ticks),
        ret_(ret),
        args_{arg1, arg2, arg3}
    {}
  };

  const FunctionTraceTemplate function_template_;
  static constexpr uint32_t max_records_ = 1000000;
  std::vector<Record> records_;
  SRWLOCK record_lock_;
  uint32_t call_count_;
  const void *function_target_;
  void *function_trampoline_;
  void *function_detour_;

  FunctionTracePack(void *target)
    : record_lock_(SRWLOCK_INIT),
      call_count_(0),
      function_target_(target),
      function_trampoline_(target),
      function_detour_(nullptr) {
    records_.reserve(max_records_);
  }

  bool ActivateDetourInternal(ExecutablePages &exec_pages) {
    function_detour_ = exec_pages.Push(function_template_,
                                       function_target_);
    bool ret = DetourAttachHelper(function_trampoline_, function_detour_);
    if (ret) {
      ret = function_template_.Fill(
        function_detour_,
        this,
        &FunctionTracePack::Push,
        function_trampoline_);
    }
    if (!ret && function_detour_) {
      exec_pages.Revert(function_detour_);
    }
    return ret;
  }

  bool DeactivateDetourInternal(ExecutablePages&) {
    return DetourDetachHelper(function_trampoline_, function_detour_);
  }

  void Print() const {
    Log(L"[FunctionTracePack %p] %d calls (%zd records stored at %p %p-1)\n",
        function_target_,
        call_count_,
        records_.size(),
        records_.begin(),
        records_.end());
  }
};
/*
void *FunctionTracePack_Start(void *arg1,
                              void *arg2,
                              void *arg3,
                              void *arg4,
                              void *arg5,
                              void *arg6,
                              void *arg7,
                              void *arg8) {
  auto instance = reinterpret_cast<FunctionTracePack*>(0xaabbccdd11223344);
  auto trampoline = reinterpret_cast<decltype(&FunctionTracePack_Start)>(0xddbbccaa44332211);
  auto rsp = reinterpret_cast<void**>(0x8877665544332211);
  uint64_t t = __rdtscp(0);
  auto ret = trampoline(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  instance->Push(__rdtscp(0) - t,
                 rsp,
                 arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
  return ret;
}
*/
void FunctionTrace(Package *package) {
  auto target = hex_to_uint64(package->args);
  if (!target) {
    Log(L"Invalid address!\n");
    return;
  }
  if (auto new_pack = std::make_unique<FunctionTracePack>(
        reinterpret_cast<void*>(target))) {
    AcquireSRWLockExclusive(&g_shim_lock);
    if (new_pack->ActivateDetour(g_exec_pages)) {
      g_packs.emplace_back(std::move(new_pack));
    }
    ReleaseSRWLockExclusive(&g_shim_lock);
  }
  WaitAndThenCleanup();
}
