#include <windows.h>

void Log(LPCWSTR format, ...);

void CheckPolicy() {
  PROCESS_MITIGATION_DYNAMIC_CODE_POLICY dynamic_code;
  PROCESS_MITIGATION_BINARY_SIGNATURE_POLICY signature;
  PROCESS_MITIGATION_IMAGE_LOAD_POLICY image_load;
  if (!GetProcessMitigationPolicy(GetCurrentProcess(),
                                  ProcessDynamicCodePolicy,
                                  &dynamic_code,
                                  sizeof(dynamic_code))
      || !GetProcessMitigationPolicy(GetCurrentProcess(),
                                     ProcessSignaturePolicy,
                                     &signature,
                                     sizeof(signature))
      || !GetProcessMitigationPolicy(GetCurrentProcess(),
                                     ProcessImageLoadPolicy,
                                     &image_load,
                                     sizeof(image_load))) {
    Log(L"GetProcessMitigationPolicy failed - %08x\n", GetLastError());
    return;
  }

  Log(L"ProcessDynamicCodePolicy\n"
      L"    ProhibitDynamicCode      %d\n"
      L"    AllowThreadOptOut        %d\n"
      L"    AllowRemoteDowngrade     %d\n"
      L"    AuditProhibitDynamicCode %d\n",
      dynamic_code.ProhibitDynamicCode,
      dynamic_code.AllowThreadOptOut,
      dynamic_code.AllowRemoteDowngrade,
      dynamic_code.AuditProhibitDynamicCode);
  Log(L"ProcessSignaturePolicy\n"
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
  Log(L"ProcessImageLoadPolicy\n"
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
}
