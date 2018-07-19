#include <windows.h>
#include <vector>
#include <memory>
#include "../../common.h"
#include "event.h"
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);

ExecutablePages g_exec_pages;
std::vector<std::unique_ptr<CodePack>> g_packs;
SRWLOCK g_shim_lock = SRWLOCK_INIT;
Event g_instance_control;

void WaitAndThenCleanup() {
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

void EndAllShims(Package*) {
  Log(L"Sending a signal to terminate shimming..\n");
  g_instance_control.Signal();
}

void PrintAllShims(Package*) {
  for (auto &pack : g_packs) {
    pack->Print();
  }
}
