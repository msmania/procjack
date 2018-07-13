#include <windows.h>
#include <detours.h>
#include <vector>
#include <memory>
#include "../../common.h"
#include "event.h"
#include "page.h"

void Log(LPCWSTR format, ...);
std::pair<uint64_t, uint64_t> address_range(char *str);

struct MeasurementPack {
  uint64_t Couter_Total;
  uint64_t Couter_Local;
  void *MeasurementStart_Trampoline;
  void *MeasurementStart_Detour;
  void *MeasurementEnd_Trampoline;
  void *MeasurementEnd_Detour;
};

CodeTemplate Template_MeasurementStart({
  // Counter_Local:        +0c
  // InjectionPoint_Start: +1a
  0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20, 0x48,
  0x09, 0xd0, 0x48, 0xba, 0x00, 0xa0, 0xa2, 0x4e,
  0xfe, 0x7f, 0x00, 0x00, 0x48, 0x89, 0x02, 0x5a,
  0x48, 0xb8, 0x50, 0xb1, 0xa2, 0x4e, 0xfe, 0x7f,
  0x00, 0x00, 0xff, 0x20,
});

CodeTemplate Template_MeasurementEnd({
  // Counter_Local:      +0c
  // Counter_Total:      +19
  // InjectionPoint_End: +29
  0x52, 0x0f, 0x31, 0x48, 0xc1, 0xe2, 0x20, 0x48,
  0x09, 0xd0, 0x48, 0xba, 0x00, 0xa0, 0xa2, 0x4e,
  0xfe, 0x7f, 0x00, 0x00, 0x48, 0x2b, 0x02, 0x48,
  0xba, 0x48, 0xb1, 0xa2, 0x4e, 0xfe, 0x7f, 0x00,
  0x00, 0xf0, 0x48, 0x0f, 0xc1, 0x02, 0x5a, 0x48,
  0xb8, 0x58, 0xb1, 0xa2, 0x4e, 0xfe, 0x7f, 0x00,
  0x00, 0xff, 0x20,
});

const WCHAR event_instance_control[] = L"clove_event_instance_control";
ExecutablePage exec_page(4096);
std::vector<std::unique_ptr<MeasurementPack>> packs;

bool DetourAttachHelper(const CodeTemplate &code,
                        void *&detour_target,
                        void *&detour_destination) {
  detour_destination = exec_page.Push(code);
  if (!detour_target) {
    Log(L"No enough space to store the code!");
    return false;
  }

  PDETOUR_TRAMPOLINE trampoline;
  PVOID target, detour;
  LONG status = DetourAttachEx(&detour_target,
                               detour_destination,
                               &trampoline,
                               &target,
                               &detour);
  if (status == NO_ERROR) {
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

bool DetourDetachHelper(void *&detour_target,
                        void *&detour_destination) {
  LONG status = DetourDetach(&detour_target, detour_destination);
  if (status != NO_ERROR) {
    Log(L"DetourDetach(%p, %p) failed with %08x\n",
        &detour_target,
        detour_destination,
        status);
  }
  return status == NO_ERROR;
}

void StartShim(Package *package) {
  static uint32_t s_RunningInstance = 0;
  auto incremented = InterlockedIncrement(&s_RunningInstance);
  // Not allow StartShim to be run in more than one thread
  if (incremented > 1) return;

  Event instance_control(/*manualReset*/FALSE,
                         /*true_to_set_signaled_initially*/FALSE,
                         event_instance_control);

  auto range = address_range(package->args);
  if (range.first == 0) {
    Log(L"Invalid range specified!\n");
    return;
  }

  auto new_pack = std::make_unique<MeasurementPack>();

  // Save the target addresses in the pack
  new_pack->MeasurementStart_Trampoline = reinterpret_cast<void*>(range.first);
  new_pack->MeasurementEnd_Trampoline = reinterpret_cast<void*>(range.second);

  // embed the parameters in the code template
  Template_MeasurementStart.Put<void*>(0x0c, &new_pack->Couter_Local);
  Template_MeasurementStart.Put<void*>(0x1a, &new_pack->MeasurementStart_Trampoline);
  Template_MeasurementEnd.Put<void*>(0x0c, &new_pack->Couter_Local);
  Template_MeasurementEnd.Put<void*>(0x19, &new_pack->Couter_Total);
  Template_MeasurementEnd.Put<void*>(0x29, &new_pack->MeasurementEnd_Trampoline);

  LONG status = DetourTransactionBegin();
  if (status != NO_ERROR) {
    Log(L"DetourTransactionBegin failed with %08x\n", status);
    goto exit;
  }
  if (DetourAttachHelper(Template_MeasurementStart,
                         new_pack->MeasurementStart_Trampoline,
                         new_pack->MeasurementStart_Detour)
      && DetourAttachHelper(Template_MeasurementEnd,
                            new_pack->MeasurementEnd_Trampoline,
                            new_pack->MeasurementEnd_Detour)) {
    packs.emplace_back(std::move(new_pack));
    status = DetourTransactionCommit();
    if (status != NO_ERROR) {
      Log(L"DetourTransactionCommit failed with %08x\n", status);
      goto exit;
    }
  }
  else {
    DetourTransactionAbort();
    Log(L"Aborted transaction.\n");
    goto exit;
  }

  // Keep the injected DLL loaded while shim is active.
  Log(L"[%04x] Detour transaction is done.  Waiting..\n", GetCurrentThreadId());
  instance_control.Wait(INFINITE);

  status = DetourTransactionBegin();
  if (status != NO_ERROR) {
    Log(L"DetourTransactionBegin failed with %08x\n", status);
    goto exit;
  }
  for (auto &pack : packs) {
    DetourDetachHelper(pack->MeasurementStart_Trampoline,
                       pack->MeasurementStart_Detour);
    DetourDetachHelper(pack->MeasurementEnd_Trampoline,
                       pack->MeasurementEnd_Detour);
  }
  status = DetourTransactionCommit();
  if (status != NO_ERROR) {
    Log(L"DetourTransactionCommit failed with %08x\n", status);
    goto exit;
  }

exit:
  InterlockedDecrement(&s_RunningInstance);
  Log(L"Goodbye!\n");
}

void EndShim(Package*) {
  Event instance_control(/*manualReset*/FALSE,
                         /*true_to_set_signaled_initially*/FALSE,
                         event_instance_control);
  if (!instance_control.WasNewlyCreated()) {
    Log(L"Sending a signal to terminate shimming..\n");
    instance_control.Signal();
  }
}

void ListShims(Package*) {
  for (int i = 0; i < packs.size(); ++i) {
    Log(L"[%02d] %llu MeasureStart: %p MeasureEnd: %p\n",
        i,
        packs[i]->Couter_Total,
        packs[i]->MeasurementStart_Detour,
        packs[i]->MeasurementEnd_Detour);
  }
}
