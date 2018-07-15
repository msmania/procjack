#include <windows.h>
#include <vector>
#include <memory>
#include "../../common.h"
#include "event.h"
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);
std::pair<uint64_t, uint64_t> address_range(char *str);
std::unique_ptr<CodePack>
  Create_MeasurementPack(void *MeasurementStart,
                         void *MeasurementEnd);

ExecutablePages g_exec_pages;
std::vector<std::unique_ptr<CodePack>> g_packs;
SRWLOCK g_shim_lock = SRWLOCK_INIT;
Event g_instance_control;

void StartShim(Package *package) {
  auto range = address_range(package->args);
  if (range.first == 0 || range.second - range.first < 5) {
    Log(L"Invalid range specified!\n");
    return;
  }

  if (auto new_pack = Create_MeasurementPack(
        reinterpret_cast<void*>(range.first),
        reinterpret_cast<void*>(range.second))) {
    AcquireSRWLockExclusive(&g_shim_lock);
    if (new_pack->ActivateDetour(g_exec_pages)) {
      g_packs.emplace_back(std::move(new_pack));
    }
    ReleaseSRWLockExclusive(&g_shim_lock);
  }

  // A thread who created the event is responsible to disarm Detour.
  // Other threads run through and exit this function.
  if (g_instance_control.CreateIfNotCreatedYet(
        /*manualReset*/FALSE,
        /*true_to_set_signaled_initially*/FALSE,
        /*name*/nullptr)) {
    // Keep the injected DLL loaded while shim is active.
    Log(L"[%04x] Waiting..\n", GetCurrentThreadId());
    g_instance_control.Wait(INFINITE);

    for (auto &pack : g_packs) {
      pack->DeactivateDetour(g_exec_pages);
    }
    Log(L"Cleanup is done.  Goodbye!\n");
  }
}

void EndShim(Package*) {
  Log(L"Sending a signal to terminate shimming..\n");
  g_instance_control.Signal();
}

void ListShims(Package*) {
  for (auto &pack : g_packs) {
    pack->Print();
  }
}
