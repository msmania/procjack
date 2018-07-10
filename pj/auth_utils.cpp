#include <windows.h>
#include <authz.h>
#include <sddl.h>
#include <aclapi.h>
#include <stdio.h>

#define LOGERROR wprintf

// https://docs.microsoft.com/en-us/windows/desktop/api/aclapi/nf-aclapi-geteffectiverightsfromacla
static DWORD GetAccessRights(PSID Sid, PSECURITY_DESCRIPTOR Sd) {
  AUTHZ_RESOURCE_MANAGER_HANDLE ResourceManager = nullptr;
  LUID UnusedId = { 0, 0 };
  AUTHZ_CLIENT_CONTEXT_HANDLE ClientContext = nullptr;
  AUTHZ_ACCESS_REQUEST AccessRequest;
  AUTHZ_ACCESS_REPLY AccessReply;
  ACCESS_MASK GrantedAccessMask = 0;
  DWORD AccessCheckError = 0;

  if (!AuthzInitializeResourceManager(AUTHZ_RM_FLAG_NO_AUDIT,
                                      /*pfnDynamicAccessCheck*/nullptr,
                                      /*pfnComputeDynamicGroups*/nullptr,
                                      /*pfnFreeDynamicGroups*/nullptr,
                                      /*szResourceManagerName*/nullptr,
                                      &ResourceManager)) {
    LOGERROR(L"AuthzInitializeRemoteResourceManager failed - %08x\n", GetLastError());
    goto cleanup;
  }

  if (!AuthzInitializeContextFromSid(/*Flags*/0,
                                     /*UserSid*/Sid,
                                     ResourceManager,
                                     /*pExpirationTime*/nullptr,
                                     UnusedId,
                                     /*DynamicGroupArgs*/nullptr,
                                     &ClientContext)) {
    LOGERROR(L"AuthzInitializeContextFromSid failed - %08x\n", GetLastError());
    goto cleanup;
  }

  ZeroMemory(&AccessRequest, sizeof(AccessRequest));
  AccessRequest.DesiredAccess = MAXIMUM_ALLOWED;
  AccessRequest.PrincipalSelfSid = nullptr;
  AccessRequest.ObjectTypeList = nullptr;
  AccessRequest.ObjectTypeListLength = 0;
  AccessRequest.OptionalArguments = nullptr;

  ZeroMemory(&AccessReply, sizeof(AccessReply));
  AccessReply.ResultListLength = 1;
  AccessReply.GrantedAccessMask = &GrantedAccessMask;
  AccessReply.Error = &AccessCheckError;

  if (!AuthzAccessCheck(/*Flags*/0,
                        ClientContext,
                        &AccessRequest,
                        /*hAuditEvent*/nullptr,
                        Sd,
                        /*OptionalSecurityDescriptorArray*/nullptr,
                        /*OptionalSecurityDescriptorCount*/0,
                        &AccessReply,
                        /*phAccessCheckResults*/nullptr)) {
    LOGERROR(L"AuthzAccessCheck failed - %08x\n", GetLastError());
    goto cleanup;
  }

cleanup:
  if (ClientContext) AuthzFreeContext(ClientContext);
  if (ResourceManager) AuthzFreeResourceManager(ResourceManager);

  return AccessCheckError;
}

// https://docs.microsoft.com/en-us/windows/desktop/SecAuthZ/modifying-the-acls-of-an-object-in-c--
//
// DACL for "C:\Program Files" contains "(A;;0x1200a9;;;AC)(A;OICIIO;GXGR;;;AC)"
// Ace[0]
//     AceType:       0x00 (ACCESS_ALLOWED_ACE_TYPE)
//     AceFlags:      0x00
//     AceSize:       0x0018
//     Access Mask:   0x001200a9
//                         READ_CONTROL
//                         SYNCHRONIZE
//                         Others(0x000000a9)
//     Ace Sid:       S-1-15-2-1
// Ace[1]
//     AceType:       0x00 (ACCESS_ALLOWED_ACE_TYPE)
//     AceFlags:      0x0b
//                        OBJECT_INHERIT_ACE
//                        CONTAINER_INHERIT_ACE
//                        INHERIT_ONLY_ACE
//     AceSize:       0x0018
//     Access Mask:   0xa0000000
//                         GENERIC_EXECUTE
//                         GENERIC_READ
//     Ace Sid:       S-1-15-2-1
//
// Account: ALL APPLICATION PACKAGES
// Domain:  APPLICATION PACKAGE AUTHORITY
// SID:     S-1-15-2-1
// Type:    SidTypeWellKnownGroup
//
bool AddPermissionForAppContainer(LPCWSTR Filename) {
  bool Ret = false;
  DWORD Status = ERROR_SUCCESS;
  PACL OldDacl = nullptr;
  PACL NewDacl = nullptr;
  PSECURITY_DESCRIPTOR Sd = nullptr;
  EXPLICIT_ACCESS NewAce;
  PSID AppContainerSid = nullptr;

  // SetNamedSecurityInfo requires non-const string.
  WCHAR FileNameCopy[MAX_PATH];
  if (!GetFullPathName(Filename, MAX_PATH, FileNameCopy, nullptr)) {
    LOGERROR(L"GetFullPathName failed - %08x\n", Status);
    goto cleanup;
  }

  Status = GetNamedSecurityInfo(
    FileNameCopy,
    SE_FILE_OBJECT,
    DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION, // AuthzAccessCheck requires Owner
    /*ppsidOwner*/nullptr,
    /*ppsidGroup*/nullptr,
    &OldDacl,
    /*ppSacl*/nullptr,
    &Sd);
  if (Status != ERROR_SUCCESS) {
    LOGERROR(L"GetNamedSecurityInfo failed - %08x\n", Status);
    goto cleanup;
  }

  if (!ConvertStringSidToSid(L"AC", &AppContainerSid)) {
    LOGERROR(L"ConvertStringSidToSid failed - %08x\n", GetLastError());
    goto cleanup;
  }

  if (GetAccessRights(AppContainerSid, Sd) != ERROR_SUCCESS) {
    LOGERROR(L"AppContainer process does not have permission to load the file. Adding an ACE..\n");

    ZeroMemory(&NewAce, sizeof(NewAce));
    NewAce.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
    NewAce.grfAccessMode = GRANT_ACCESS;
    NewAce.grfInheritance = NO_INHERITANCE;
    NewAce.Trustee.pMultipleTrustee = nullptr;
    NewAce.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    NewAce.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    NewAce.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    NewAce.Trustee.ptstrName = (LPWSTR)AppContainerSid;

    Status = SetEntriesInAcl(1, &NewAce, OldDacl, &NewDacl);
    if (Status != ERROR_SUCCESS) {
      LOGERROR(L"SetEntriesInAcl failed - %08x\n", Status);
      goto cleanup;
    }

    Status = SetNamedSecurityInfo(FileNameCopy,
                                  SE_FILE_OBJECT,
                                  DACL_SECURITY_INFORMATION,
                                  /*psidOwner*/nullptr,
                                  /*psidGroup*/nullptr,
                                  NewDacl,
                                  /*pSacl*/nullptr);
    if (Status != ERROR_SUCCESS) {
      LOGERROR(L"SetNamedSecurityInfo failed - %08x\n", Status);
      goto cleanup;
    }
  }

  Ret = true;

cleanup:
  if (NewDacl) LocalFree(NewDacl);
  if (AppContainerSid) LocalFree(AppContainerSid);
  if (Sd) LocalFree(Sd);
  return Ret;
}
