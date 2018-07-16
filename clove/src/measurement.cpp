#include <windows.h>
#include <memory>
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);

struct MeasurementPack final : public CodePack {
#ifdef _WIN64
  static constexpr uint8_t Template_Measurement[] = {
    // Start
    0x50, 0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20,
    0x48, 0x09, 0xd0, 0x48, 0xba, 0x00, 0x90, 0x3b,
    0x72, 0xfd, 0x7f, 0x00, 0x00, 0x48, 0x89, 0x02,
    0x5a, 0x58, 0xe9, 0x73, 0x56, 0x34, 0x12,
    // End
    0x50, 0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20,
    0x48, 0x09, 0xd0, 0x48, 0xba, 0x00, 0x90, 0x3b,
    0x72, 0xfd, 0x7f, 0x00, 0x00, 0x48, 0x2b, 0x02,
    0x48, 0xba, 0x08, 0x90, 0x3b, 0x72, 0xfd, 0x7f,
    0x00, 0x00, 0xf0, 0x48, 0x0f, 0xc1, 0x02, 0x5a,
    0x58, 0xe9, 0x73, 0x56, 0x34, 0x12,

  };
  static constexpr uint32_t offset_Measurement_S_Counter_Local = 0x0d;
  static constexpr uint32_t offset_Measurement_S_FinalJump = 0x1a;
  static constexpr uint32_t offset_Measurement_E = 0x1f;
  static constexpr uint32_t offset_Measurement_E_Counter_Local = 0x2c;
  static constexpr uint32_t offset_Measurement_E_Counter_Total = 0x39;
  static constexpr uint32_t offset_Measurement_E_FinalJump = 0x48;
#else
  static constexpr uint8_t Template_Measurement[] = {
    // Start
    0x50, 0x52, 0x56, 0xbe, 0x00, 0x10, 0x7b, 0x5c,
    0x0f, 0x31, 0x89, 0x06, 0x89, 0x56, 0x04, 0x5e,
    0x5a, 0x58, 0xe9, 0x73, 0x56, 0x34, 0x12,
    // End
    0x81, 0xec, 0x0c, 0x00, 0x00, 0x00, 0x50, 0x53,
    0x51, 0x52, 0x55, 0x56, 0x57, 0x0f, 0x31, 0x89,
    0xc5, 0xc7, 0x44, 0x24, 0x20, 0x08, 0x10, 0x7b,
    0x5c, 0xb9, 0x00, 0x10, 0x7b, 0x5c, 0x2b, 0x29,
    0x89, 0xd0, 0x1b, 0x41, 0x04, 0x89, 0x6c, 0x24,
    0x24, 0x89, 0x44, 0x24, 0x1c, 0xb9, 0x08, 0x10,
    0x7b, 0x5c, 0x8b, 0x39, 0x89, 0xfb, 0x8b, 0x71,
    0x04, 0x01, 0xeb, 0x89, 0xf1, 0x89, 0xf2, 0x11,
    0xc1, 0x89, 0xf8, 0x8b, 0x6c, 0x24, 0x20, 0xf0,
    0x0f, 0xc7, 0x4d, 0x00, 0x8b, 0x6c, 0x24, 0x24,
    0x39, 0xf8, 0x8b, 0x44, 0x24, 0x1c, 0x75, 0xd5,
    0x39, 0xf2, 0x75, 0xd1, 0x5f, 0x5e, 0x5d, 0x5a,
    0x59, 0x5b, 0x58, 0x81, 0xc4, 0x0c, 0x00, 0x00,
    0x00, 0xe9, 0x73, 0x56, 0x34, 0x12,
  };
  static constexpr uint32_t offset_Measurement_S_Counter_Local = 0x04;
  static constexpr uint32_t offset_Measurement_S_FinalJump = 0x12;
  static constexpr uint32_t offset_Measurement_E = 0x17;
  static constexpr uint32_t offset_Measurement_E_Counter_Local = 0x31;
  static constexpr uint32_t offset_Measurement_E_Counter_Total1 = 0x2c;
  static constexpr uint32_t offset_Measurement_E_Counter_Total2 = 0x45;
  static constexpr uint32_t offset_Measurement_E_FinalJump = 0x80;
#endif

  uint64_t Couter_Total;
  uint64_t Couter_Local;
  const void *MeasurementStart_Target;
  void *MeasurementStart_Trampoline;
  void *MeasurementStart_Detour;
  const void *MeasurementEnd_Target;
  void *MeasurementEnd_Trampoline;
  void *MeasurementEnd_Detour;

  MeasurementPack(void *MeasurementStart,
                  void *MeasurementEnd)
    : Couter_Total(0),
      Couter_Local(0),
      MeasurementStart_Target(MeasurementStart),
      MeasurementStart_Trampoline(MeasurementStart),
      MeasurementStart_Detour(nullptr),
      MeasurementEnd_Target(MeasurementEnd),
      MeasurementEnd_Trampoline(MeasurementEnd),
      MeasurementEnd_Detour(nullptr)
  {}

  bool ActivateDetourInternal(ExecutablePages &exec_pages) {
    MeasurementStart_Detour = exec_pages.Push(*this, MeasurementStart_Target);
    MeasurementEnd_Detour = at<void>(MeasurementStart_Detour,
                                     offset_Measurement_E);
    bool ret = DetourAttachHelper(MeasurementStart_Trampoline,
                                  MeasurementStart_Detour)
               && DetourAttachHelper(MeasurementEnd_Trampoline,
                                     MeasurementEnd_Detour);
    if (ret) {
      ret = PutImmediateNearJump(at<void>(MeasurementStart_Detour,
                                          offset_Measurement_S_FinalJump),
                                 MeasurementStart_Trampoline)
            && PutImmediateNearJump(at<void>(MeasurementStart_Detour,
                                             offset_Measurement_E_FinalJump),
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

  size_t Size() const {
    return sizeof(Template_Measurement);
  }

  void Print() const {
    Log(L"[Measurement %p-%p] %llu\n",
        MeasurementStart_Target,
        MeasurementEnd_Target,
        Couter_Total);
  }

  void CopyTo(uint8_t *destination) const {
    std::memcpy(destination,
                Template_Measurement,
                sizeof(Template_Measurement));
    *at<const void*>(destination, offset_Measurement_S_Counter_Local)
      = *at<const void*>(destination, offset_Measurement_E_Counter_Local)
        = &Couter_Local;
#ifdef _WIN64
    *at<const void*>(destination, offset_Measurement_E_Counter_Total)
      = &Couter_Total;
#else
    *at<const void*>(destination, offset_Measurement_E_Counter_Total1)
      = *at<const void*>(destination, offset_Measurement_E_Counter_Total2)
        = &Couter_Total;
#endif
  }
};

std::unique_ptr<CodePack>
  Create_MeasurementPack(void *MeasurementStart,
                         void *MeasurementEnd) {
  return std::make_unique<MeasurementPack>(MeasurementStart,
                                           MeasurementEnd);
}
