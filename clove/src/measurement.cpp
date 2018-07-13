#include <windows.h>
#include <memory>
#include "page.h"

void Log(LPCWSTR format, ...);

template<typename T>
static T *at(void *base, uint32_t offset) {
  return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(base) + offset);
}

struct MeasurementPack final : public CodePack {
  static constexpr uint8_t Template_MeasurementStart[] = {
    0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20, 0x48,
    0x09, 0xd0, 0x48, 0xba, 0x00, 0xa0, 0xa2, 0x4e,
    0xfe, 0x7f, 0x00, 0x00, 0x48, 0x89, 0x02, 0x5a,
    0x48, 0xb8, 0x50, 0xb1, 0xa2, 0x4e, 0xfe, 0x7f,
    0x00, 0x00, 0xff, 0x20,
  };
  static constexpr uint32_t offset_MeasurementStart_Counter_Local = 0x0c;
  static constexpr uint32_t offset_MeasurementStart_InjectionPoint_Start = 0x1a;
  static constexpr uint8_t Template_MeasurementEnd[] = {
    0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20, 0x48,
    0x09, 0xd0, 0x48, 0xba, 0x00, 0xa0, 0xa2, 0x4e,
    0xfe, 0x7f, 0x00, 0x00, 0x48, 0x2b, 0x02, 0x48,
    0xba, 0x48, 0xb1, 0xa2, 0x4e, 0xfe, 0x7f, 0x00,
    0x00, 0xf0, 0x48, 0x0f, 0xc1, 0x02, 0x5a, 0x48,
    0xb8, 0x58, 0xb1, 0xa2, 0x4e, 0xfe, 0x7f, 0x00,
    0x00, 0xff, 0x20,
  };
  static constexpr uint32_t offset_MeasurementEnd_Counter_Local = 0x0c;
  static constexpr uint32_t offset_MeasurementEnd_Counter_Total = 0x19;
  static constexpr uint32_t offset_MeasurementEnd_InjectionPoint_End = 0x29;

  uint64_t Couter_Total;
  uint64_t Couter_Local;
  void *MeasurementStart_Trampoline;
  void *MeasurementStart_Detour;
  void *MeasurementEnd_Trampoline;
  void *MeasurementEnd_Detour;

  MeasurementPack(void *Target_MeasurementStart,
                  void *Target_MeasurementEnd)
    : Couter_Total(0),
      Couter_Local(0),
      MeasurementStart_Trampoline(Target_MeasurementStart),
      MeasurementStart_Detour(nullptr),
      MeasurementEnd_Trampoline(Target_MeasurementEnd),
      MeasurementEnd_Detour(nullptr)
  {}

  bool ActivateDetourInternal(ExecutablePage &exec_page) {
    MeasurementStart_Detour = exec_page.Push(*this);
    MeasurementEnd_Detour = at<void>(MeasurementStart_Detour,
                                     sizeof(Template_MeasurementStart)
                                     + 1);
    return DetourAttachHelper(MeasurementStart_Trampoline,
                              MeasurementStart_Detour)
           && DetourAttachHelper(MeasurementEnd_Trampoline,
                                 MeasurementEnd_Detour);
  }

  bool DeactivateDetourInternal(ExecutablePage&) {
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
    Log(L"%llu MeasureStart: %p MeasureEnd: %p\n",
        Couter_Total,
        MeasurementStart_Detour,
        MeasurementEnd_Detour);
  }

  void CopyTo(uint8_t *destination) const {
    std::memcpy(destination,
                Template_MeasurementStart,
                sizeof(Template_MeasurementStart));
    *at<const void*>(destination, offset_MeasurementStart_Counter_Local) = &Couter_Local;
    *at<const void*>(destination, offset_MeasurementStart_InjectionPoint_Start)
      = &MeasurementStart_Trampoline;

    *at<uint8_t>(destination, sizeof(Template_MeasurementStart)) = 0xCC;

    destination += (sizeof(Template_MeasurementStart) + 1);
    std::memcpy(destination,
                Template_MeasurementEnd,
                sizeof(Template_MeasurementEnd));
    *at<const void*>(destination, offset_MeasurementEnd_Counter_Local) = &Couter_Local;
    *at<const void*>(destination, offset_MeasurementEnd_Counter_Total) = &Couter_Total;
    *at<const void*>(destination, offset_MeasurementEnd_InjectionPoint_End)
      = &MeasurementEnd_Trampoline;
  }
};

std::unique_ptr<CodePack>
  Create_MeasurementPack(void *Target_MeasurementStart,
                         void *Target_MeasurementEnd) {
  return std::make_unique<MeasurementPack>(Target_MeasurementStart,
                                           Target_MeasurementEnd);
}
