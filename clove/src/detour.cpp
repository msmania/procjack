#include <windows.h>
#include <detours.h>
#include <cstdint>
#include <utility>
#include "../../common.h"
#include "event.h"

void Log(LPCWSTR format, ...);
std::pair<uint64_t, uint64_t> address_range(char *str);

extern "C" {
  size_t Counter_Total = 0;
  void *InjectionPoint_Start = nullptr;
  void *InjectionPoint_End = nullptr;
  void Clove_Start();
  void Clove_End();
}

const WCHAR event_instance_control[] = L"clove_event_instance_control";

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

  InjectionPoint_Start = reinterpret_cast<void*>(range.first);
  InjectionPoint_End = reinterpret_cast<void*>(range.second);

  PDETOUR_TRAMPOLINE trampoline;
  PVOID target, detour;

  LONG status = DetourTransactionBegin();
  status = DetourAttachEx(&InjectionPoint_Start,
                          Clove_Start,
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
    Log(L"DetourAttach#1(%p, %p) failed with %08x\n",
        &InjectionPoint_Start,
        Clove_Start,
        status);
    goto exit;
  }
  status = DetourAttachEx(&InjectionPoint_End,
                          Clove_End,
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
    Log(L"DetourAttach#2(%p, %p) failed with %08x\n",
        &InjectionPoint_End,
        Clove_End,
        status);
    goto exit;
  }
  status = DetourTransactionCommit();
  if (status != NO_ERROR) {
    Log(L"DetourTransactionCommit failed with %08x\n", status);
    goto exit;
  }

  // Keep the injected DLL loaded while shim is active.
  Log(L"[%04x] Waiting for the signal..\n", GetCurrentThreadId());
  instance_control.Wait(INFINITE);

  status = DetourTransactionBegin();
  if (status != NO_ERROR) {
    Log(L"DetourTransactionCommit failed with %08x\n", status);
    goto exit;
  }
  status = DetourDetach(&InjectionPoint_Start, Clove_Start);
  if (status != NO_ERROR) {
    Log(L"DetourDetach#1(%p, %p) failed with %08x\n",
        &InjectionPoint_Start,
        Clove_Start,
        status);
    goto exit;
  }
  status = DetourDetach(&InjectionPoint_End, Clove_End);
  if (status != NO_ERROR) {
    Log(L"DetourDetach#1(%p, %p) failed with %08x\n",
        &InjectionPoint_Start,
        Clove_Start,
        status);
    goto exit;
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
