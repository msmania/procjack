#include <windows.h>
#include <detours.h>
#include <cstdint>
#include "pack.h"

void Log(LPCWSTR format, ...);

CodePack::~CodePack()
{}

bool CodePack::DetourAttachHelper(void *&detour_target,
                                  void *detour_destination) {
  PDETOUR_TRAMPOLINE trampoline;
  PVOID target, detour;
  LONG status = DetourAttachEx(&detour_target,
                               detour_destination,
                               &trampoline,
                               &target,
                               &detour);
  if (status == NO_ERROR) {
    // Detours updates the pointer at the moment of commit,
    // but we need a trampoline address right now.
    detour_target = trampoline;
    Log(L"Detouring: %p --> %p (trampoline:%p)\n",
        target,
        detour,
        trampoline);
  }
  else {
    Log(L"DetourAttach(%p, %p) failed with %08x\n",
        &detour_target,
        detour_destination,
        status);
  }
  return status == NO_ERROR;
}

bool CodePack::DetourDetachHelper(void *&detour_target,
                                  void *detour_destination) {
  LONG status = DetourDetach(&detour_target, detour_destination);
  if (status != NO_ERROR) {
    Log(L"DetourDetach(%p, %p) failed with %08x\n",
        &detour_target,
        detour_destination,
        status);
  }
  return status == NO_ERROR;
}

bool CodePack::ActivateDetour(ExecutablePages &exec_pages) {
  return DetourTransaction<&CodePack::ActivateDetourInternal>(exec_pages);
}

bool CodePack::DeactivateDetour(ExecutablePages &exec_pages) {
  return DetourTransaction<&CodePack::DeactivateDetourInternal>(exec_pages);
}

const void *CodePack::PutImmediateNearJump(void *jump_from,
                                           const void *jump_to) {
  const auto next_instruction = at<uint8_t>(jump_from, 5);
  const int64_t delta64 = at<const uint8_t>(jump_to, 0) - next_instruction;
  if (delta64 > 0x7fffffff || delta64 < 0xffffffff80000000i64) {
    Log(L"Cannot make a near jump.\n");
    return nullptr;
  }

  if (jump_from) {
    *at<uint8_t>(jump_from, 0) = 0xe9;
    *at<int32_t>(jump_from, 1) = static_cast<int32_t>(delta64);
  }
  return next_instruction;
}
