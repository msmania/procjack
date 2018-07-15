#include <windows.h>
#include <memory>
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);

struct MeasurementPack final : public CodePack {
  static constexpr uint8_t Template_MeasurementStart[] = {
    0x50, 0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20,
    0x48, 0x09, 0xd0, 0x48, 0xba, 0x00, 0xa0, 0xa2,
    0x4e, 0xfe, 0x7f, 0x00, 0x00, 0x48, 0x89, 0x02,
    0x5a, 0x58,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, // placeholder for jmp rel32
  };
  static constexpr uint32_t offset_MeasurementStart_Counter_Local = 0x0d;
  static constexpr uint32_t offset_MeasurementStart_FinalJump = 0x1a;
  static constexpr uint8_t Template_MeasurementEnd[] = {
    0x50, 0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20,
    0x48, 0x09, 0xd0, 0x48, 0xba, 0x00, 0xa0, 0xa2,
    0x4e, 0xfe, 0x7f, 0x00, 0x00, 0x48, 0x2b, 0x02,
    0x48, 0xba, 0x48, 0xb1, 0xa2, 0x4e, 0xfe, 0x7f,
    0x00, 0x00, 0xf0, 0x48, 0x0f, 0xc1, 0x02, 0x5a,
    0x58,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc,  // placeholder for jmp rel32
  };
  static constexpr uint32_t offset_MeasurementEnd_Counter_Local = 0x0d;
  static constexpr uint32_t offset_MeasurementEnd_Counter_Total = 0x1a;
  static constexpr uint32_t offset_MeasurementEnd_FinalJump = 0x29;

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
                                     sizeof(Template_MeasurementStart)
                                     + 1);
    bool ret = DetourAttachHelper(MeasurementStart_Trampoline,
                                  MeasurementStart_Detour)
               && DetourAttachHelper(MeasurementEnd_Trampoline,
                                     MeasurementEnd_Detour);
    if (ret) {
      ret = PutImmediateNearJump(at<void>(MeasurementStart_Detour,
                                          offset_MeasurementStart_FinalJump),
                                 MeasurementStart_Trampoline)
            && PutImmediateNearJump(at<void>(MeasurementEnd_Detour,
                                             offset_MeasurementEnd_FinalJump),
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
    return sizeof(Template_MeasurementStart)
           + sizeof(Template_MeasurementEnd)
           + 1;
  }

  void Print() const {
    Log(L"[Measurement %p-%p] %llu\n",
        MeasurementStart_Target,
        MeasurementEnd_Target,
        Couter_Total);
  }

  void CopyTo(uint8_t *destination) const {
    std::memcpy(destination,
                Template_MeasurementStart,
                sizeof(Template_MeasurementStart));
    *at<const void*>(destination, offset_MeasurementStart_Counter_Local)
      = &Couter_Local;

    *at<uint8_t>(destination, sizeof(Template_MeasurementStart)) = 0xCC;

    destination += (sizeof(Template_MeasurementStart) + 1);
    std::memcpy(destination,
                Template_MeasurementEnd,
                sizeof(Template_MeasurementEnd));
    *at<const void*>(destination, offset_MeasurementEnd_Counter_Local)
      = &Couter_Local;
    *at<const void*>(destination, offset_MeasurementEnd_Counter_Total)
      = &Couter_Total;
  }
};

std::unique_ptr<CodePack>
  Create_MeasurementPack(void *MeasurementStart,
                         void *MeasurementEnd) {
  return std::make_unique<MeasurementPack>(MeasurementStart,
                                           MeasurementEnd);
}
