#include <windows.h>
#include <vector>
#include <memory>
#include <numeric>
#include "../../common.h"
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);
std::vector<uint64_t> address_chain(const char *str);
void WaitAndThenCleanup();

extern "C" {
  extern uint8_t MeasurementChain_Start;
  extern uint8_t MeasurementChain_Checkpoint;
  extern uint8_t MeasurementChain_End;
}

extern ExecutablePages g_exec_pages;
extern std::vector<std::unique_ptr<CodePack>> g_packs;
extern SRWLOCK g_shim_lock;
/*
void M_Start() {
  ++(*reinterpret_cast<uint32_t*>(0x8ffffffffffffff0));
  *reinterpret_cast<uint64_t*>(0x8ffffffffffffff8) = __rdtsc();
}

void M_Checkpoint() {
  auto t = __rdtsc();
  reinterpret_cast<uint64_t*>(0x8fffffffffffffe0)[0x0] +=
    (t - *reinterpret_cast<uint64_t*>(0x8ffffffffffffff8));
  *reinterpret_cast<uint64_t*>(0x8ffffffffffffff8) = t;
  ++(*reinterpret_cast<uint32_t*>(0x8ffffffffffffff0));
}
*/
struct MeasurementChainPack final : public CodePack {
  class Template_Start : public CodeTemplate {
#ifdef _WIN64
    static constexpr uint32_t offset_CallCount = 0x04;
    static constexpr uint32_t offset_Checkpoint = 0x19;
#else
    static constexpr uint32_t offset_CallCount = 0x03;
    static constexpr uint32_t offset_Checkpoint_L = 0x0c;
    static constexpr uint32_t offset_Checkpoint_H = 0x12;
#endif

  public:
    size_t Size() const {
      return &MeasurementChain_Checkpoint - &MeasurementChain_Start;
    }

    void CopyTo(void *destination) const {
      std::memcpy(destination, &MeasurementChain_Start, Size());
    }

    bool Fill(void *start_address,
              const void *call_count,
              const void *checkpoint,
              const void *final_jump_to) const {
      *at<const void*>(start_address, offset_CallCount) = call_count;
#ifdef _WIN64
      *at<const void*>(start_address, offset_Checkpoint) = checkpoint;
#else
      *at<const void*>(start_address, offset_Checkpoint_L) = checkpoint;
      *at<const void*>(start_address, offset_Checkpoint_H)
        = at<const void>(checkpoint, 4);
#endif
      return PutImmediateNearJump(
        at<void>(start_address, static_cast<uint32_t>(Size() - 5)),
        final_jump_to);
    }
  };
  class Template_Checkpoint : public CodeTemplate {
#ifdef _WIN64
    static constexpr uint32_t offset_Total1 = 0x05;
    static constexpr uint32_t offset_Checkpoint1 = 0x12;
    static constexpr uint32_t offset_Total2 = 0x2b;
    static constexpr uint32_t offset_Checkpoint2 = 0x38;
    static constexpr uint32_t offset_CallCount = 0x45;
#else
    static constexpr uint32_t offset_Total1_L = 0x06;
    static constexpr uint32_t offset_Total1_H = 0x0c;
    static constexpr uint32_t offset_Checkpoint1_L = 0x12;
    static constexpr uint32_t offset_Checkpoint1_H = 0x18;
    static constexpr uint32_t offset_Total2_L = 0x22;
    static constexpr uint32_t offset_Total2_H = 0x2a;
    static constexpr uint32_t offset_Checkpoint2_L = 0x2f;
    static constexpr uint32_t offset_Checkpoint2_H = 0x35;
    static constexpr uint32_t offset_CallCount = 0x3a;
#endif

  public:
    size_t Size() const {
      return &MeasurementChain_End - &MeasurementChain_Checkpoint;
    }

    void CopyTo(void *destination) const {
      std::memcpy(destination, &MeasurementChain_Checkpoint, Size());
    }

    bool Fill(void *start_address,
              const void *call_count,
              const void *checkpoint,
              const void *total_ticks,
              const void *final_jump_to) const {
#ifdef _WIN64
      *at<const void*>(start_address, offset_Total1)
        = *at<const void*>(start_address, offset_Total2)
          = total_ticks;
      *at<const void*>(start_address, offset_Checkpoint1)
        = *at<const void*>(start_address, offset_Checkpoint2)
          = checkpoint;
#else
      *at<const void*>(start_address, offset_Total1_L)
        = *at<const void*>(start_address, offset_Total2_L)
          = total_ticks;
      *at<const void*>(start_address, offset_Total1_H)
        = *at<const void*>(start_address, offset_Total2_H)
          = at<const void>(total_ticks, 4);
      *at<const void*>(start_address, offset_Checkpoint1_L)
        = *at<const void*>(start_address, offset_Checkpoint2_L)
          = checkpoint;
      *at<const void*>(start_address, offset_Checkpoint1_H)
        = *at<const void*>(start_address, offset_Checkpoint2_H)
          = at<const void>(checkpoint, 4);
#endif
      *at<const void*>(start_address, offset_CallCount) = call_count;
      return PutImmediateNearJump(
        at<void>(start_address, static_cast<uint32_t>(Size() - 5)),
        final_jump_to);
    }
  };

  const Template_Start template_start_;
  const Template_Checkpoint template_checkpoint_;
  uint64_t checkpoint_;
  const uint32_t n_;
  std::vector<uint32_t> call_count_;
  std::vector<uint64_t> ticks_;
  std::vector<void*> function_target_;
  std::vector<void*> function_trampoline_;
  std::vector<void*> function_detour_;

  MeasurementChainPack(const std::vector<uint64_t> &address_chain)
    : checkpoint_(0),
      n_(static_cast<uint32_t>(address_chain.size())),
      call_count_(n_, 0),
      ticks_(n_, 0),
      function_target_(n_, 0),
      function_trampoline_(n_, 0),
      function_detour_(n_, 0) {
    for (uint32_t i = 0; i < n_; ++i) {
      function_target_[i]
        = function_trampoline_[i]
        = reinterpret_cast<void*>(address_chain[i]);
    }
  }

  bool ActivateDetourInternal(ExecutablePages &exec_pages) {
    if (n_ < 2) return false;

    function_detour_[0] = exec_pages.Push(template_start_,
                                          function_target_[0]);
    if (!DetourAttachHelper(function_trampoline_[0],
                           function_detour_[0])
        || !template_start_.Fill(function_detour_[0],
                                 &call_count_[0],
                                 &checkpoint_,
                                 function_trampoline_[0]))
      goto revert;

    for (uint32_t i = 1; i < n_; ++i) {
      function_detour_[i] = exec_pages.Push(template_checkpoint_,
                                            function_target_[i]);
      if (!DetourAttachHelper(function_trampoline_[i],
                             function_detour_[i])
          || !template_checkpoint_.Fill(function_detour_[i],
                                        &call_count_[i],
                                        &checkpoint_,
                                        &ticks_[i],
                                        function_trampoline_[i]))
        goto revert;
    }

    return true;

  revert:
    for (uint32_t i = 0; i < n_; ++i) {
      // Need to revert in the reverse order because only the last-pushed
      // page is revertable.
      if (function_detour_[n_ - 1 - i]
          && !exec_pages.Revert(function_detour_[n_ - 1 - i])) {
        Log(L"Failed to revert %p\n", function_detour_[n_ - 1 - i]);
      }
    }
    return false;
  }

  bool DeactivateDetourInternal(ExecutablePages&) {
    bool ret = true;
    for (uint32_t i = 0; i < n_; ++i) {
      ret &= DetourDetachHelper(function_trampoline_[i],
                                function_detour_[i]);
    }
    return ret;
  }

  void Print() const {
    Log(L"[MeasurementChain] total ticks: %llu\n",
        std::accumulate(ticks_.begin(), ticks_.end(), 0I64));
    for (uint32_t i = 1; i < n_; ++i) {
      Log(L"  %p-%p: %llu (callcount: %d %d)\n",
        function_target_[i - 1],
        function_target_[i],
        ticks_[i],
        call_count_[i - 1],
        call_count_[i]);
    }
  }
};

void MeasurementChain(Package *package) {
  auto chain = address_chain(package->nw.args);
  if (chain.size() < 2) {
    Log(L"Invalid parameter!\n");
    return;
  }
  if (auto new_pack = std::make_unique<MeasurementChainPack>(chain)) {
    AcquireSRWLockExclusive(&g_shim_lock);
    if (new_pack->ActivateDetour(g_exec_pages)) {
      g_packs.emplace_back(std::move(new_pack));
    }
    ReleaseSRWLockExclusive(&g_shim_lock);
  }
  WaitAndThenCleanup();
}
