[org 0]
[bits 32]

; pj!CInjectData::Package
;    +0x000 InitialCode      : [2048] UChar
;    +0x800 DllPath          : [260] Uint2B
;    +0xa08 peb              : Ptr32 Void
;    +0xa0c ntdll            : Ptr32 Void
;    +0xa10 kernel32         : Ptr32 Void
;    +0xa14 LoadLibraryW     : Ptr32 Void
;    +0xa18 FreeLibrary      : Ptr32 Void
;    +0xa1c GetProcAddress   : Ptr32 Void
;    +0xa20 CreateThread     : Ptr32 Void
;    +0xa24 ExitThread       : Ptr32 Void
;    +0xa28 WaitForSingleObject : Ptr32 Void
;    +0xa2c CloseHandle      : Ptr32 Void
;    +0xa30 Context          : [1] UChar

ShellCode:
mov eax,[fs:0x30]
push esi
mov esi,[esp+0x8]
push esi
mov [esi+0xa08],eax
call GetImageBase

push esi
push dword [esi+0xa0c]
call GetProcAddress

push esi
push dword [esi+0xa10]
call GetProcAddress

push    edi
lea     eax, [esi+800h]
push    eax
mov     eax, [esi+0A14h]
call    eax         ; LoadLibrary
mov     edi, eax
test    edi, edi
je      label00c114a4

mov     ecx, [esi+0A1Ch]
push    0DEADh      ; Ordinal
push    edi
call    ecx         ; GetProcAddress
test    eax, eax
je      label00c1149b

push    ebx
push    0
push    0
push    esi
push    eax
mov     eax, [esi+0A20h]
push    0
push    0
call    eax         ; CreateThread
mov     ebx, eax
test    ebx, ebx
je      label00c1149a

mov     ecx, [esi+0A28h]
push    0FFFFFFFFh
push    ebx
call    ecx         ; WaitForSingleObject
mov     eax, [esi+0A2Ch]
push    ebx
call    eax         ; CloseHandle

label00c1149a:
pop     ebx

label00c1149b:
mov     eax, [esi+0A18h]
push    edi
call    eax         ; FreeLibrary

label00c114a4:
mov     eax, [esi+0A24h]
push    0
call    eax         ; ExitThread
pop     edi

add esp,byte +0x14
mov eax,0x2a
pop esi
ret

int3
int3

GetImageBase:
push ebx
push esi
mov esi,[esp+0xc]       ; esi = Context
mov eax,[esi+0xa08]     ; eax = PEB
mov ebx,[eax+0xc]
add ebx,byte +0x14
mov ecx,[ebx]
cmp ecx,ebx
jz label0xe0

push edi
mov edi,[esi+0xa0c]

label0x23:
mov eax,[ecx+0x28]
test edi,edi
jnz label0x44

cmp dword [eax],0x74006e
jnz label0x44

cmp dword [eax+0x4],0x6c0064
jnz label0x44

cmp dword [eax+0x8],0x2e006c
jz label0x60

label0x44:
mov edx,[eax]
cmp edx,0x54004e
jnz label0x6b

cmp dword [eax+0x4],0x4c0044
jnz label0x6b

cmp dword [eax+0x8],0x2e004c
jnz label0x6b

label0x60:
mov edi,[ecx+0x10]
mov [esi+0xa0c],edi     ; package->ntdll
jmp short label0xd5

label0x6b:
cmp dword [esi+0xa10],byte +0x0
jnz label0xa0

cmp edx,0x45004b
jnz label0xa0

cmp dword [eax+0x4],0x4e0052
jnz label0xa0

cmp dword [eax+0x8],0x4c0045
jnz label0xa0

cmp dword [eax+0xc],0x320033
jnz label0xa0

cmp dword [eax+0x10],0x44002e
jz label0xcc

label0xa0:
cmp edx,0x65006b
jnz label0xd5

cmp dword [eax+0x4],0x6e0072
jnz label0xd5

cmp dword [eax+0x8],0x6c0065
jnz label0xd5

cmp dword [eax+0xc],0x320033
jnz label0xd5

cmp dword [eax+0x10],0x64002e
jnz label0xd5

label0xcc:
mov eax,[ecx+0x10]
mov [esi+0xa10],eax     ; package->kernel32

label0xd5:
mov ecx,[ecx]
cmp ecx,ebx
jnz label0x23

pop edi

label0xe0:
pop esi
pop ebx
ret


int3
int3


GetProcAddress:
sub esp,byte +0xc
mov eax,0x5a4d
push ebx
mov ebx,[esp+0x14]
cmp [ebx],ax
jnz label0x330

mov edx,[ebx+0x3c]
cmp dword [edx+ebx],0x4550
jnz label0x330

movzx ecx,word [edx+ebx+0x4]
push edi
mov edi,0x14c
cmp cx,di
jz label0x144

mov eax,0x8664
cmp cx,ax
jnz label0x32f

label0x144:
cmp cx,di
push esi
mov esi,0x60
mov eax,0x70
cmovz eax,esi
add eax,edx
xor edi,edi
mov eax,[eax+ebx+0x18]
add eax,ebx
mov [esp+0xc],eax
mov ecx,[eax+0x20]
mov esi,[eax+0x24]
mov edx,[eax+0x1c]
add ecx,ebx
add esi,ebx
add edx,ebx
mov [esp+0x10],ecx
mov [esp+0x14],esi
mov [esp+0x1c],edx
cmp [eax+0x18],edi
jna label0x32e

mov edx,[esp+0x20]
push ebp
mov ebp,[edx+0xa14]

label0x192:
mov eax,[ecx+edi*4]
movzx ecx,word [esi+edi*2]
mov esi,[esp+0x20]
add eax,ebx
mov esi,[esi+ecx*4]
add esi,ebx
test ebp,ebp
jnz label0x1d5

cmp dword [eax],0x64616f4c
jnz label0x1d5

cmp dword [eax+0x4],0x7262694c
jnz label0x1d5

cmp dword [eax+0x8],0x57797261
jnz label0x1d5

cmp byte [eax+0xc],0x0
jnz label0x1d5

mov ebp,esi
mov [edx+0xa14],esi     ; package->LoadLibrary
jmp dword label0x317

label0x1d5:
cmp dword [edx+0xa18],byte +0x0
jnz label0x203

cmp dword [eax],0x65657246
jnz label0x203

cmp dword [eax+0x4],0x7262694c
jnz label0x203

cmp dword [eax+0x8],0x797261
jnz label0x203

mov [edx+0xa18],esi     ; package->FreeLibrary
jmp dword label0x317

label0x203:
cmp dword [edx+0xa1c],byte +0x0
jnz label0x242

cmp dword [eax],0x50746547
jnz label0x242

cmp dword [eax+0x4],0x41636f72
jnz label0x242

cmp dword [eax+0x8],0x65726464
jnz label0x242

mov ecx,[eax+0xc]
and ecx,0xffffff
cmp ecx,0x7373
jnz label0x242

mov [edx+0xa1c],esi     ; package->GetProcAddress
jmp dword label0x317

label0x242:
cmp dword [edx+0xa20],byte +0x0
jnz label0x276

cmp dword [eax],0x61657243
jnz label0x276

cmp dword [eax+0x4],0x68546574
jnz label0x276

cmp dword [eax+0x8],0x64616572
jnz label0x276

cmp byte [eax+0xc],0x0
jnz label0x276

mov [edx+0xa20],esi     ; package->CreateThread
jmp dword label0x317

label0x276:
cmp dword [edx+0xa24],byte +0x0
jnz label0x2b1

cmp dword [eax],0x456c7452
jnz label0x2b1

cmp dword [eax+0x4],0x55746978
jnz label0x2b1

cmp dword [eax+0x8],0x54726573
jnz label0x2b1

cmp dword [eax+0xc],0x61657268
jnz label0x2b1

cmp word [eax+0x10],byte +0x64
jnz label0x2b1

mov [edx+0xa24],esi     ; package->ExitThread
jmp short label0x317

label0x2b1:
cmp dword [edx+0xa28],byte +0x0
jnz label0x2ee

cmp dword [eax],0x74696157
jnz label0x2ee

cmp dword [eax+0x4],0x53726f46
jnz label0x2ee

cmp dword [eax+0x8],0x6c676e69
jnz label0x2ee

cmp dword [eax+0xc],0x6a624f65
jnz label0x2ee

cmp dword [eax+0x10],0x746365
jnz label0x2ee

mov [edx+0xa28],esi     ; package->WaitForSingleObject
jmp short label0x317

label0x2ee:
cmp dword [edx+0xa2c],byte +0x0
jnz label0x317

cmp dword [eax],0x736f6c43
jnz label0x317

cmp dword [eax+0x4],0x6e614865
jnz label0x317

cmp dword [eax+0x8],0x656c64
jnz label0x317

mov [edx+0xa2c],esi     ; package->CloseHandle

label0x317:
mov eax,[esp+0x10]
mov ecx,[esp+0x14]
mov esi,[esp+0x18]
inc edi
cmp edi,[eax+0x18]
jc label0x192

pop ebp

label0x32e:
pop esi

label0x32f:
pop edi

label0x330:
pop ebx
add esp,byte +0xc
ret


int3
int3
