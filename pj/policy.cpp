#ifndef DOWNLEVEL

#include <windows.h>
#include <stdio.h>

#define LOGERROR wprintf
#define LOGINFO LOGERROR

// https://github.com/processhacker/processhacker/blob/master/phnt/include/ntpsapi.h
struct PROCESS_MITIGATION_POLICY_INFORMATION {
  PROCESS_MITIGATION_POLICY Policy;
  union {
    PROCESS_MITIGATION_ASLR_POLICY ASLRPolicy;
    PROCESS_MITIGATION_STRICT_HANDLE_CHECK_POLICY StrictHandleCheckPolicy;
    PROCESS_MITIGATION_SYSTEM_CALL_DISABLE_POLICY SystemCallDisablePolicy;
    PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY ExtensionPointDisablePolicy;
    PROCESS_MITIGATION_DYNAMIC_CODE_POLICY DynamicCodePolicy;
    PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY ControlFlowGuardPolicy;
    PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY SignaturePolicy;
    PROCESS_MITIGATION_FONT_DISABLE_POLICY FontDisablePolicy;
    PROCESS_MITIGATION_IMAGE_LOAD_POLICY ImageLoadPolicy;
    PROCESS_MITIGATION_SYSTEM_CALL_FILTER_POLICY SystemCallFilterPolicy;
    PROCESS_MITIGATION_PAYLOAD_RESTRICTION_POLICY PayloadRestrictionPolicy;
    PROCESS_MITIGATION_CHILD_PROCESS_POLICY ChildProcessPolicy;
  };
};

bool DisablePolicy(HANDLE target) {
  PROCESS_MITIGATION_DYNAMIC_CODE_POLICY dynamic_code;
  PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY signature;
  PROCESS_MITIGATION_IMAGE_LOAD_POLICY image_load;
  if (!GetProcessMitigationPolicy(target,
                                  ProcessDynamicCodePolicy,
                                  &dynamic_code,
                                  sizeof(dynamic_code))
      || !GetProcessMitigationPolicy(target,
                                     ProcessSignaturePolicy,
                                     &signature,
                                     sizeof(signature))
      || !GetProcessMitigationPolicy(target,
                                     ProcessImageLoadPolicy,
                                     &image_load,
                                     sizeof(image_load)))
    return false;

  LOGINFO(L"ProcessDynamicCodePolicy\n"
          L"    ProhibitDynamicCode      %d\n"
          L"    AllowThreadOptOut        %d\n"
          L"    AllowRemoteDowngrade     %d\n"
          L"    AuditProhibitDynamicCode %d\n",
          dynamic_code.ProhibitDynamicCode,
          dynamic_code.AllowThreadOptOut,
          dynamic_code.AllowRemoteDowngrade,
          dynamic_code.AuditProhibitDynamicCode);
  LOGINFO(L"ProcessSignaturePolicy\n"
          L"    MicrosoftSignedOnly      %d\n"
          L"    StoreSignedOnly          %d\n"
          L"    MitigationOptIn          %d\n"
          L"    AuditMicrosoftSignedOnly %d\n"
          L"    AuditStoreSignedOnly     %d\n",
          signature.MicrosoftSignedOnly,
          signature.StoreSignedOnly,
          signature.MitigationOptIn,
          signature.AuditMicrosoftSignedOnly,
          signature.AuditStoreSignedOnly);
  LOGINFO(L"ProcessImageLoadPolicy\n"
          L"    NoRemoteImages                 %d\n"
          L"    NoLowMandatoryLabelImages      %d\n"
          L"    PreferSystem32Images           %d\n"
          L"    AuditNoRemoteImages            %d\n"
          L"    AuditNoLowMandatoryLablImages  %d\n",
          image_load.NoRemoteImages,
          image_load.NoLowMandatoryLabelImages,
          image_load.PreferSystem32Images,
          image_load.AuditNoRemoteImages,
          image_load.AuditNoLowMandatoryLabelImages);

  // Try to disable the policy only when ProhibitDynamicCode is on.
  if (!dynamic_code.ProhibitDynamicCode)
    return false;

  bool ret = false;
  if (auto NtSetInformationProcess =
             reinterpret_cast<DWORD(WINAPI*)(HANDLE, DWORD, PVOID, ULONG)>(
               GetProcAddress(GetModuleHandle(L"ntdll.dll"),
                              "NtSetInformationProcess"))) {
    PROCESS_MITIGATION_POLICY_INFORMATION info{};
    info.Policy = ProcessDynamicCodePolicy;
    DWORD status = NtSetInformationProcess(target,
                                           /*ProcessMitigationPolicy*/0x34,
                                           &info,
                                           sizeof(info));
    ret = status == 0;
    if (status) {
      LOGERROR(L"NtSetInformationProcess(ProcessDynamicCodePolicy) failed - %08x\n",
               status);
    }
    info = {};
    info.Policy = ProcessSignaturePolicy;
    status = NtSetInformationProcess(target,
                                     /*ProcessMitigationPolicy*/0x34,
                                     &info,
                                     sizeof(info));
    ret = status == 0;
    if (status) {
      LOGERROR(L"NtSetInformationProcess(ProcessSignaturePolicy) failed - %08x\n",
               status);
    }
    info = {};
    info.Policy = ProcessImageLoadPolicy;
    status = NtSetInformationProcess(target,
                                     /*ProcessMitigationPolicy*/0x34,
                                     &info,
                                     sizeof(info));
    ret = status == 0;
    if (status) {
      LOGERROR(L"NtSetInformationProcess(ProcessImageLoadPolicy) failed - %08x\n",
               status);
    }
  }
  return ret;
}

#endif