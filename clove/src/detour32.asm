BITS 32

section .text

MeasurementChain_Start:
  push eax
  push edx
  mov  eax, 0xFFFFFFF0        ; callcount
  inc  dword [eax]
  rdtsc
  mov  dword [0xFFFFFFF8],eax ; checkpoint
  mov  dword [0xFFFFFFFC],edx
  pop  edx
  pop  eax
  jmp  $ + 0x12345678

MeasurementChain_Checkpoint:
  push    eax
  push    ecx
  push    edx
  push    esi
  mov     esi,dword [7E0h]        ; total
  mov     ecx,dword [7E4h]
  sub     esi,dword [0FFFFFFF8h]  ; checkpoint
  sbb     ecx,dword [0FFFFFFFCh]
  rdtsc
  add     esi,eax
  mov     dword [7E0h],esi        ; total
  adc     ecx,edx
  mov     dword [7E4h],ecx
  mov     dword [0xFFFFFFF8],eax  ; checkpoint
  mov     dword [0xFFFFFFFC],edx
  mov     eax, 0xFFFFFFF0         ; callcount
  inc     dword [eax]
  pop     esi
  pop     edx
  pop     ecx
  pop     eax
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
