BITS 32

global _MeasurementChain_Start
global _MeasurementChain_Checkpoint
global _MeasurementChain_End
global _FunctionTracePack_Start
global _FunctionTracePack_End

section .text

_MeasurementChain_Start:
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

_MeasurementChain_Checkpoint:
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
_MeasurementChain_End:

_FunctionTracePack_Start:
  sub     esp,14h
  push    esi

  mov dword [esp+10h], ecx
  mov dword [esp+14h], edx

  rdtscp
  mov     dword [esp+04h],eax
  mov     dword [esp+08h],edx

  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  mov     ecx, dword [esp+20h+10h]
  mov     edx, dword [esp+20h+14h]
  mov     eax,44332211h     ; trampoline
  call    eax
  add     esp,20h
  mov     dword [esp+0Ch],eax

  rdtscp

  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]
  push    dword [esp+38h]

  ; Push the original ECX as 1st argument regardless of calling convention.
  ; This makes the 8th argument on the stack invisible from the callee,
  ; but it's just fine.
  lea     ecx, [esp+20h+10h]
  push    dword [ecx]

  add     ecx, 8            ; original esp
  push    ecx

  sub     eax,dword [esp+28h+04h]
  sbb     edx,dword [esp+28h+08h]
  push    edx
  push    eax

  mov     ecx,11223344h     ; pack
  mov     eax,55667788h     ; push
  call    eax

  ; Need to unwind the 8th argument
  add     esp, 4

  mov     eax,dword [esp+0Ch]

  pop     esi
  add     esp,14h
  ret
_FunctionTracePack_End:
