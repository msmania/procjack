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

struct CPU_CONTEXT {
#ifdef _WIN64
  uint64_t rax,
           rbx,
           rcx,
           rdx,
           rsi,
           rdi,
           rsp,
           rbp,
           r8,
           r9,
           r10,
           r11,
           r12,
           r13,
           r14,
           r15;
#else
  uint32_t edi,
           esi,
           ebp,
           esp,
           ebx,
           edx,
           ecx,
           eax;
#endif
};

struct FunctionTracePack final : public CodePack {
  __declspec(noinline)
  void Push(const CPU_CONTEXT &context) {
    InterlockedIncrement(&call_count_);
    AcquireSRWLockExclusive(&record_lock_);
    if (records_.size() < max_records_) {
#ifdef _WIN64
      const void *ret = *reinterpret_cast<void**>(context.rsp);
#else
      const void *ret = *reinterpret_cast<void**>(context.esp);
#endif
      records_.emplace_back(ret);
    }
    ReleaseSRWLockExclusive(&record_lock_);
#if 0
#ifdef _WIN64
    Log(L"%S: %d\n"
        L"rax=%016llx rbx=%016llx rcx=%016llx\n"
        L"rdx=%016llx rsi=%016llx rdi=%016llx\n"
        L"rsp=%016llx rbp=%016llx\n"
        L" r8=%016llx  r9=%016llx r10=%016llx\n"
        L"r11=%016llx r12=%016llx r13=%016llx\n"
        L"r14=%016llx r15=%016llx\n",
        __FUNCTION__,
        call_count_,
        context.rax,
        context.rbx,
        context.rcx,
        context.rdx,
        context.rsi,
        context.rdi,
        context.rsp,
        context.rbp,
        context.r8,
        context.r9,
        context.r10,
        context.r11,
        context.r12,
        context.r13,
        context.r14,
        context.r15);
#else
    Log(L"%S: %d\n"
        "eax=%08x ebx=%08x ecx=%08x edx=%08x esi=%08x edi=%08x\n"
        "esp=%08x ebp=%08x\n",
        __FUNCTION__,
        call_count_,
        context.eax,
        context.ebx,
        context.ecx,
        context.edx,
        context.esi,
        context.edi,
        context.esp,
        context.ebp);
#endif
#endif
  }

  class FunctionTraceTemplate : public CodeTemplate {
#ifdef _WIN64
    static constexpr uint32_t offset_FunctionTracePack = 0x63;
    static constexpr uint32_t offset_FunctionTracePackPush = 0x6d;
#else
    static constexpr uint32_t offset_FunctionTracePack = 0x05;
    static constexpr uint32_t offset_FunctionTracePackPush = 0x0a;
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
      *at<const void*>(start_address, offset_FunctionTracePack) = pack;
      *at<decltype(func_to_push)>(start_address, offset_FunctionTracePackPush)
        = func_to_push;
      return PutImmediateNearJump(
        at<void>(start_address, static_cast<uint32_t>(Size() - 5)),
        trampoline);
    }
  };

  struct Record {
    const uint32_t tid_;
    const void *ret_;
    Record(const void *ret)
      : tid_(GetCurrentThreadId()),
        ret_(ret)
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
