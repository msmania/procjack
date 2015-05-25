[org 0]
[bits 32]

; 0:000> dt expeb!Package
;    +0x000 InitialCode      : [2048] UChar
;    +0x800 DllPath          : [260] Uint2B
;    +0xa08 peb              : Ptr32 _PEB
;    +0xa0c kernel32         : Ptr32 Void
;    +0xa10 xxxLoadLibrary   : Ptr32 Void
;    +0xa14 xxxFreeLibrary   : Ptr32 Void
;    +0xa18 xxxGetProcAddress : Ptr32 Void
;    +0xa1c Context          : [1] UChar
;
ShellCode:
sub     esp,10h
mov     eax,dword [fs:00000030h]
push    ebx
push    esi
mov     esi,dword [esp+1Ch]
push    edi
push    esi
mov     dword [esp+10h],123h
xor     edi,edi
mov     dword [esp+14h],0
mov     dword [esp+18h],456h
mov     dword [esp+1Ch],0
mov     dword [esi+0A08h],eax
call    GetImageBase
push    esi
push    dword [esi+0A0Ch]
call    GetProcAddress
add     esp,0Ch
lea     eax,[esi+800h]
push    eax
mov     eax,dword [esi+0A10h]
call    eax        ; LoadLibrary
mov     ebx,eax
test    ebx,ebx
je      ShellCode_Exit

mov     eax,dword [esi+0A18h]
push    0DEADh
push    ebx
call    eax        ; GetProcAddress
test    eax,eax
je      ShellCode_FreeLibrary

push    esi
call    eax
mov     edi,eax

ShellCode_FreeLibrary:
mov     ecx,dword [esi+0A14h]
push    ebx
call    ecx        ; FreeLibrary

ShellCode_Exit:
lea     eax,[esp+0Ch]
mov     dword [eax], 74697845h
mov     dword [eax+4h], 65726854h
mov     dword [eax+8h], 00006461h
push    eax
push    dword [esi+0A0Ch]
mov     eax,dword [esi+0A18h]
call    eax        ; GetProcAddress("ExitThread")
mov     edi,eax

lea     eax,[esp+0Ch]
mov     dword [eax], 74726956h
mov     dword [eax+4h], 466c6175h
mov     dword [eax+8h], 00656572h
push    eax
push    dword [esi+0A0Ch]
mov     eax,dword [esi+0A18h]
call    eax        ; GetProcAddress("VirtualAlloc")

push    8000h
push    0
push    esi

push    edi
jmp     eax

int3

GetImageBase:
push    ebx
mov     ebx,dword [esp+8]
push    edi
mov     eax,dword [ebx+0A08h]
mov     edi,dword [eax+0Ch]
add     edi,14h
mov     edx,dword [edi]
cmp     edx,edi
je      GetImageBase_Exit

push    esi
mov     esi,dword [ebx+0A0Ch]
nop

GetImageBase_Loop:
mov     eax,dword [edx+28h]
test    esi,esi
jne     GetImageBase_Lower

cmp     dword [eax],45004Bh
jne     GetImageBase_Lower

cmp     dword [eax+4],4E0052h
jne     GetImageBase_Lower

cmp     dword [eax+8],4C0045h
jne     GetImageBase_Lower

cmp     dword [eax+0Ch],320033h
jne     GetImageBase_Lower

cmp     word [eax+10h],2Eh
je      GetImageBase_Found

GetImageBase_Lower:
cmp     dword [eax],65006Bh
jne     GetImageBase_Continue

cmp     dword [eax+4],6E0072h
jne     GetImageBase_Continue

cmp     dword [eax+8],6C0065h
jne     GetImageBase_Continue

cmp     dword [eax+0Ch],320033h
jne     GetImageBase_Continue

cmp     word [eax+10h],2Eh
jne     GetImageBase_Continue

GetImageBase_Found:
mov     esi,dword [edx+10h]
mov     dword [ebx+0A0Ch],esi

GetImageBase_Continue:
mov     edx,dword [edx]
cmp     edx,edi
jne     GetImageBase_Loop

pop     esi

GetImageBase_Exit:
pop     edi
pop     ebx
ret

int3

GetProcAddress:
sub     esp,8
mov     eax,5A4Dh
push    edi
mov     edi,dword [esp+10h]
cmp     word [edi],ax
jne     GetProcAddress_Exit

mov     edx,dword [edi+3Ch]
cmp     dword [edx+edi],4550h
jne     GetProcAddress_Exit

movzx   ecx,word [edx+edi+4]
push    ebx
mov     ebx,14Ch
cmp     cx,bx
je      GetProcAddress_Loop

mov     eax,8664h
cmp     cx,ax
jne     GetProcAddress_Exit_ebx

GetProcAddress_Loop:
push    ebp
cmp     cx,bx
mov     eax,70h
push    esi
mov     esi,60h
cmove   eax,esi
xor     esi,esi
add     eax,edx
mov     ebp,dword [eax+edi+18h]
mov     eax,dword [ebp+edi+20h]
add     ebp,edi
add     eax,edi
mov     dword [esp+10h],eax
mov     ecx,dword [ebp+24h]
mov     edx,dword [ebp+1Ch]
add     ecx,edi
add     edx,edi
mov     dword [esp+14h],ecx
mov     dword [esp+1Ch],edx
cmp     dword [ebp+18h],esi
jbe     GetProcAddress_LoopEnd

mov     edx,dword [esp+20h]
mov     ebx,dword [edx+0A10h]
nop

GetProcAddress_LoopStart:
movzx   ecx,word [ecx+esi*2]
mov     edx,dword [esp+1Ch]
mov     eax,dword [eax+esi*4]
add     eax,edi
mov     ecx,dword [edx+ecx*4]
mov     edx,dword [esp+20h]
add     ecx,edi
test    ebx,ebx
jne     GetProcAddress_FreeLibrary

cmp     dword [eax],64616F4Ch
jne     GetProcAddress_FreeLibrary

cmp     dword [eax+4],7262694Ch
jne     GetProcAddress_FreeLibrary

cmp     dword [eax+8],57797261h
jne     GetProcAddress_FreeLibrary

cmp     byte [eax+0Ch],0
jne     GetProcAddress_FreeLibrary

mov     ebx,ecx
mov     dword [edx+0A10h],ecx
jmp     GetProcAddress_Continue

GetProcAddress_FreeLibrary:
cmp     dword [edx+0A14h],0
jne     GetProcAddress_GetProcAddress

cmp     dword [eax],65657246h
jne     GetProcAddress_GetProcAddress

cmp     dword [eax+4],7262694Ch
jne     GetProcAddress_GetProcAddress

cmp     dword [eax+8],797261h
jne     GetProcAddress_GetProcAddress

mov     dword [edx+0A14h],ecx
jmp     GetProcAddress_Continue

GetProcAddress_GetProcAddress:
cmp     dword [edx+0A18h],0
jne     GetProcAddress_Continue

cmp     dword [eax],50746547h
jne     GetProcAddress_Continue

cmp     dword [eax+4],41636F72h
jne     GetProcAddress_Continue

cmp     dword [eax+8],65726464h
jne     GetProcAddress_Continue

mov     eax,dword [eax+0Ch]
and     eax,0FFFFFFh
cmp     eax,7373h
jne     GetProcAddress_Continue

mov     dword [edx+0A18h],ecx

GetProcAddress_Continue:
mov     eax,dword [esp+10h]
inc     esi
mov     ecx,dword [esp+14h]
cmp     esi,dword [ebp+18h]
jb      GetProcAddress_LoopStart

GetProcAddress_LoopEnd:
pop     esi
pop     ebp

GetProcAddress_Exit_ebx:
pop     ebx

GetProcAddress_Exit:
pop     edi
add     esp,8
ret

int3
