#include <windows.h>
#include <vector>
#include <memory>
#include "../../common.h"
#include "event.h"
#include "pack.h"
#include "page.h"

void Log(LPCWSTR format, ...);
uint64_t hex_to_uint64(const char *s);
void WaitAndThenCleanup();

extern ExecutablePages g_exec_pages;
extern std::vector<std::unique_ptr<CodePack>> g_packs;
extern SRWLOCK g_shim_lock;

struct FunctionCallPack final : public CodePack {
  class CallRecordTemplate : public CodeTemplate {
#ifdef _WIN64
    static constexpr uint8_t blob[] = {
      0x50, 0x51, 0x52, 0x48, 0x81, 0xec, 0x10, 0x00,
      0x00, 0x00, 0x48, 0x8b, 0x44, 0x24, 0x28, 0x48,
      0x89, 0x04, 0x24, 0x48, 0x89, 0x4c, 0x24, 0x08,
      0x48, 0x89, 0xe2, 0x48, 0xb9, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0x07, 0x48, 0xb8, 0xfe,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xff,
      0xd0, 0x48, 0x81, 0xc4, 0x10, 0x00, 0x00, 0x00,
      0x5a, 0x59, 0x58, 0xe9, 0x73, 0x56, 0x34, 0x12,
    };
    static constexpr uint32_t offset_ThisPointer = 0x1d;
    static constexpr uint32_t offset_EntryPoint = 0x27;
#else
    static constexpr uint8_t blob[] = {
      0x50, 0x51, 0x8b, 0x44, 0x24, 0x08, 0x50, 0x54,
      0xb8, 0xff, 0xff, 0xff, 0x7f, 0x50, 0xb8, 0xfe,
      0xff, 0xff, 0x7f, 0xff, 0xd0, 0x81, 0xc4, 0x0c,
      0x00, 0x00, 0x00, 0x59, 0x58, 0xe9, 0x73, 0x56,
      0x34, 0x12,
    };
    static constexpr uint32_t offset_ThisPointer = 0x9;
    static constexpr uint32_t offset_EntryPoint = 0xf;
#endif
    static constexpr uint32_t offset_FinalJump = sizeof(blob) - 5;

  public:
    size_t Size() const {
      return sizeof(blob);
    }

    void CopyTo(void *destination) const {
      std::memcpy(destination, blob, sizeof(blob));
    }

    bool Fill(void *start_address,
              const FunctionCallPack *object,
              const void *entrypoint_function,
              const void *final_jump_to) const {
      *at<const void*>(start_address, offset_ThisPointer) = object;
      *at<const void*>(start_address, offset_EntryPoint) = entrypoint_function;
      return PutImmediateNearJump(at<void>(start_address, offset_FinalJump),
                                  final_jump_to);
    }
  };

  struct Record {
    void *ret_ = nullptr;
    void *first_arg_ = nullptr;
    Record(void *ret, void *first_arg)
      : ret_(ret),
        first_arg_(first_arg)
    {}
  };

  static void EntryPoint(void *this_pointer, void **args) {
    reinterpret_cast<FunctionCallPack*>(this_pointer)->RegisterRecord(args);
  }

  void RegisterRecord(void **args) {
    // args[0] = return address
    // args[1] = 1st argument
    ++call_count_;
    if (records_.size() < max_records_)
      records_.emplace_back(args[0], args[1]);
  }

  const CallRecordTemplate function_template_;
  static constexpr uint32_t max_records_ = 1000000;
  std::vector<Record> records_;
  uint32_t call_count_;
  const void *function_target_;
  void *function_trampoline_;
  void *function_detour_;

  FunctionCallPack(void *target)
    : call_count_(0),
      function_target_(target),
      function_trampoline_(target),
      function_detour_(nullptr) {
    records_.reserve(max_records_);
  }

  bool ActivateDetourInternal(ExecutablePages &exec_pages) {
    function_detour_ = exec_pages.Push(function_template_,
                                       function_target_);
    bool ret = DetourAttachHelper(function_trampoline_, function_detour_);
    if (ret) {
      ret = function_template_.Fill(function_detour_,
                                    this,
                                    EntryPoint,
                                    function_trampoline_);
    }
    if (!ret && function_detour_) {
      exec_pages.Revert(function_detour_);
    }
    return ret;
  }

  bool DeactivateDetourInternal(ExecutablePages&) {
    return DetourDetachHelper(function_trampoline_, function_detour_);
  }

  void Print() const {
    Log(L"[FunctionCallPack %p] %d calls (%zd records stored at %p-%p)\n",
        function_target_,
        call_count_,
        records_.size(),
        records_.begin(),
        records_.end());
  }
};

void RecordCalls(Package *package) {
  auto target = hex_to_uint64(package->args);
  if (!target) {
    Log(L"Invalid address!\n");
    return;
  }
  if (auto new_pack = std::make_unique<FunctionCallPack>(
        reinterpret_cast<void*>(target))) {
    AcquireSRWLockExclusive(&g_shim_lock);
    if (new_pack->ActivateDetour(g_exec_pages)) {
      g_packs.emplace_back(std::move(new_pack));
    }
    ReleaseSRWLockExclusive(&g_shim_lock);
  }
  WaitAndThenCleanup();
}
