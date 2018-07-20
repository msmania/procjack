BITS 32

section .data
Counter_Local dq 0x000042d68342622e
Counter_Total dq 0x71000000ffffffff

section .text

Measure_Start:
  push eax
  push edx
  push esi
  mov esi, Counter_Local
  rdtsc
  mov dword [esi],eax
  mov dword [esi+0x4],edx
  pop esi
  pop edx
  pop eax
  ; ret
  jmp $ + 0x12345678

Measure_End:
  sub     esp,0Ch
  push    eax
  push    ebx
  push    ecx
  push    edx
  push    ebp
  push    esi
  push    edi
  rdtsc
  mov     ebp,eax
  mov     dword [esp+20h], Counter_Total
  mov     ecx, Counter_Local
  sub     ebp,dword [ecx]
  mov     eax,edx
  sbb     eax,dword [ecx+0x4]
  mov     dword [esp+24h],ebp
  mov     dword [esp+1Ch],eax

label_0x30:
  mov     ecx, Counter_Total
  mov     edi,dword [ecx]
  mov     ebx,edi
  mov     esi,dword [ecx+0x4]
  add     ebx,ebp
  mov     ecx,esi
  mov     edx,esi
  adc     ecx,eax
  mov     eax,edi
  mov     ebp,dword [esp+20h] ; Counter_Total
  lock cmpxchg8b [ebp]
  mov     ebp,dword [esp+24h]
  cmp     eax,edi
  mov     eax,dword [esp+1Ch]
  jne     label_0x30

  cmp     edx,esi
  jne     label_0x30

  pop     edi
  pop     esi
  pop     ebp
  pop     edx
  pop     ecx
  pop     ebx
  pop     eax
  add     esp,0Ch
  ; ret
  jmp $ + 0x12345678

FunctionCallPack_Start:
  push eax
  push ecx ; 1st argument (depending on a calling convention though)

  mov eax, [esp + 8] ; return address
  push eax
  push esp

  mov eax, 0x7fffffff ; FunctionCallPack instance
  push eax
  mov eax, 0x7ffffffe ; EntryPoint
  call eax

  add esp, 0Ch

  pop ecx
  pop eax
  jmp $ + 0x12345678
