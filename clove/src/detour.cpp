#include <windows.h>
#include <vector>
#include <memory>
#include "../../common.h"
#include "event.h"
#include "page.h"

void Log(LPCWSTR format, ...);
std::pair<uint64_t, uint64_t> address_range(char *str);

std::unique_ptr<CodePack>
  Create_MeasurementPack(void *Target_MeasurementStart,
                         void *Target_MeasurementEnd);

const WCHAR event_instance_control[] = L"clove_event_instance_control";
ExecutablePage g_exec_page(4096);
std::vector<std::unique_ptr<CodePack>> g_packs;

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

  auto new_pack = Create_MeasurementPack(
    reinterpret_cast<void*>(range.first),
    reinterpret_cast<void*>(range.second));

  if (new_pack->ActivateDetour(g_exec_page)) {
    g_packs.emplace_back(std::move(new_pack));
  }

  // Keep the injected DLL loaded while shim is active.
  Log(L"[%04x] Detour transaction is done.  Waiting..\n", GetCurrentThreadId());
  instance_control.Wait(INFINITE);

  for (auto &pack : g_packs) {
    pack->DeactivateDetour(g_exec_page);
  }

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
  for (auto &pack : g_packs) {
    pack->Print();
  }
}
