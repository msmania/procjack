#include <windows.h>
#include <vector>
#include <memory>
#include "../../common.h"
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);
std::pair<uint64_t, uint64_t> address_range(char *str);
void WaitAndThenCleanup();

extern ExecutablePages g_exec_pages;
extern std::vector<std::unique_ptr<CodePack>> g_packs;
extern SRWLOCK g_shim_lock;

struct MeasurementPack final : public CodePack {
  class MeasurementStart : public CodeTemplate {
#ifdef _WIN64
    static constexpr uint8_t blob[] = {
      0x50, 0x52, 0x48, 0xb8, 0xfd, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0x07, 0xff, 0x00, 0x0f, 0x31,
      0x48, 0xc1, 0xe2, 0x20, 0x48, 0x09, 0xd0, 0x48,
      0xba, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0x07, 0x48, 0x89, 0x02, 0x5a, 0x58, 0xe9, 0x73,
      0x56, 0x34, 0x12,
    };
    static constexpr uint32_t offset_CallCount = 0x04;
    static constexpr uint32_t offset_Start_Tick = 0x19;
#else
    static constexpr uint8_t blob[] = {
      0x50, 0x52, 0x56, 0xb8, 0xfd, 0xff, 0xff, 0x7f,
      0xff, 0x00, 0xbe, 0xff, 0xff, 0xff, 0x7f, 0x0f,
      0x31, 0x89, 0x06, 0x89, 0x56, 0x04, 0x5e, 0x5a,
      0x58, 0xe9, 0x73, 0x56, 0x34, 0x12,
    };
    static constexpr uint32_t offset_CallCount = 0x04;
    static constexpr uint32_t offset_Start_Tick = 0x0b;
#endif
    static constexpr uint32_t offset_FinalJump = sizeof(blob) - 5;

  public:
    size_t Size() const {
      return sizeof(blob);
    }

    void CopyTo(void *destination) const {
      std::memcpy(destination, blob, sizeof(blob));
    }

    bool Fill(void *start_address,
              const void *call_count,
              const void *tick_start,
              const void *final_jump_to) {
      *at<const void*>(start_address, offset_CallCount) = call_count;
      *at<const void*>(start_address, offset_Start_Tick) = tick_start;
      return PutImmediateNearJump(at<void>(start_address, offset_FinalJump),
                                  final_jump_to);
    }
  };
  class MeasurementEnd : public CodeTemplate {
#ifdef _WIN64
    static constexpr uint8_t blob[] = {
      0x50, 0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20,
      0x48, 0x09, 0xd0, 0x48, 0xba, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0x07, 0x48, 0x2b, 0x02,
      0x48, 0xba, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0x07, 0xf0, 0x48, 0x0f, 0xc1, 0x02, 0x48,
      0xb8, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0x07, 0xff, 0x00, 0x5a, 0x58, 0xe9, 0x73, 0x56,
      0x34, 0x12,
    };
    static constexpr uint32_t offset_Start_Tick = 0x0d;
    static constexpr uint32_t offset_Total_Ticks = 0x1a;
    static constexpr uint32_t offset_CallCount = 0x29;
#else
    static constexpr uint8_t blob[] = {
      0x81, 0xec, 0x0c, 0x00, 0x00, 0x00, 0x50, 0x53,
      0x51, 0x52, 0x55, 0x56, 0x57, 0x0f, 0x31, 0x89,
      0xc5, 0xc7, 0x44, 0x24, 0x20, 0xfe, 0xff, 0xff,
      0x7f, 0xb9, 0xff, 0xff, 0xff, 0x7f, 0x2b, 0x29,
      0x89, 0xd0, 0x1b, 0x41, 0x04, 0x89, 0x6c, 0x24,
      0x24, 0x89, 0x44, 0x24, 0x1c, 0xb9, 0xfe, 0xff,
      0xff, 0x7f, 0x8b, 0x39, 0x89, 0xfb, 0x8b, 0x71,
      0x04, 0x01, 0xeb, 0x89, 0xf1, 0x89, 0xf2, 0x11,
      0xc1, 0x89, 0xf8, 0x8b, 0x6c, 0x24, 0x20, 0xf0,
      0x0f, 0xc7, 0x4d, 0x00, 0x8b, 0x6c, 0x24, 0x24,
      0x39, 0xf8, 0x8b, 0x44, 0x24, 0x1c, 0x75, 0xd5,
      0x39, 0xf2, 0x75, 0xd1, 0xb8, 0xfd, 0xff, 0xff,
      0x7f, 0xff, 0x00, 0x5f, 0x5e, 0x5d, 0x5a, 0x59,
      0x5b, 0x58, 0x81, 0xc4, 0x0c, 0x00, 0x00, 0x00,
      0xe9, 0x73, 0x56, 0x34, 0x12,
    };
    static constexpr uint32_t offset_Total_Ticks1 = 0x15;
    static constexpr uint32_t offset_Start_Tick = 0x1a;
    static constexpr uint32_t offset_Total_Ticks2 = 0x2e;
    static constexpr uint32_t offset_CallCount = 0x5d;
#endif
    static constexpr uint32_t offset_FinalJump = sizeof(blob) - 5;

  public:
    size_t Size() const {
      return sizeof(blob);
    }

    void CopyTo(void *destination) const {
      std::memcpy(destination, blob, sizeof(blob));
    }

    bool Fill(void *start_address,
              const void *call_count,
              const void *tick_start,
              const void *total_ticks,
              const void *final_jump_to) {
      *at<const void*>(start_address, offset_CallCount) = call_count;
      *at<const void*>(start_address, offset_Start_Tick) = tick_start;
#ifdef _WIN64
      *at<const void*>(start_address, offset_Total_Ticks) = total_ticks;
#else
      *at<const void*>(start_address, offset_Total_Ticks1)
        = *at<const void*>(start_address, offset_Total_Ticks2)
          = total_ticks;
#endif
      return PutImmediateNearJump(at<void>(start_address, offset_FinalJump),
                                  final_jump_to);
    }
  };


  MeasurementStart function_start_template_;
  MeasurementEnd function_end_template_;
  uint32_t start_call_count_;
  uint32_t end_call_count_;
  uint64_t total_ticks_;
  uint64_t tick_start_;
  const void *MeasurementStart_Target;
  void *MeasurementStart_Trampoline;
  void *MeasurementStart_Detour;
  const void *MeasurementEnd_Target;
  void *MeasurementEnd_Trampoline;
  void *MeasurementEnd_Detour;

  MeasurementPack(void *MeasurementStart,
                  void *MeasurementEnd)
    : start_call_count_(0),
      end_call_count_(0),
      total_ticks_(0),
      tick_start_(0),
      MeasurementStart_Target(MeasurementStart),
      MeasurementStart_Trampoline(MeasurementStart),
      MeasurementStart_Detour(nullptr),
      MeasurementEnd_Target(MeasurementEnd),
      MeasurementEnd_Trampoline(MeasurementEnd),
      MeasurementEnd_Detour(nullptr)
  {}

  bool ActivateDetourInternal(ExecutablePages &exec_pages) {
    MeasurementStart_Detour = exec_pages.Push(function_start_template_,
                                              MeasurementStart_Target);
    MeasurementEnd_Detour = exec_pages.Push(function_end_template_,
                                            MeasurementEnd_Target);
    bool ret = DetourAttachHelper(MeasurementStart_Trampoline,
                                  MeasurementStart_Detour)
               && DetourAttachHelper(MeasurementEnd_Trampoline,
                                     MeasurementEnd_Detour);
    if (ret) {
      ret = function_start_template_.Fill(MeasurementStart_Detour,
                                          &start_call_count_,
                                          &tick_start_,
                                          MeasurementStart_Trampoline)
            && function_end_template_.Fill(MeasurementEnd_Detour,
                                           &end_call_count_,
                                           &tick_start_,
                                           &total_ticks_,
                                           MeasurementEnd_Trampoline);
      if (!ret) {
        exec_pages.Revert(MeasurementStart_Detour);
      }
    }
    return ret;
  }

  bool DeactivateDetourInternal(ExecutablePages&) {
    return DetourDetachHelper(MeasurementStart_Trampoline,
                              MeasurementStart_Detour)
           && DetourDetachHelper(MeasurementEnd_Trampoline,
                                 MeasurementEnd_Detour);
  }

  void Print() const {
    Log(L"[Measurement %p-%p] %llu (callcount: %d %d)\n",
        MeasurementStart_Target,
        MeasurementEnd_Target,
        total_ticks_,
        start_call_count_,
        end_call_count_);
  }
};

void StartMeasurement(Package *package) {
  auto range = address_range(package->args);
  if (range.first == 0 || range.second - range.first < 5) {
    Log(L"Invalid range specified!\n");
    return;
  }

  if (auto new_pack = std::make_unique<MeasurementPack>(
        reinterpret_cast<void*>(range.first),
        reinterpret_cast<void*>(range.second))) {
    AcquireSRWLockExclusive(&g_shim_lock);
    if (new_pack->ActivateDetour(g_exec_pages)) {
      g_packs.emplace_back(std::move(new_pack));
    }
    ReleaseSRWLockExclusive(&g_shim_lock);
  }

  WaitAndThenCleanup();
}
