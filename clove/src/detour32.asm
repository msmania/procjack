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
  pushad
  mov     eax, esp
  push    eax
  mov     ecx,11223344h   ; this
  mov     eax,55667788h   ; func
  call    eax
  popad
  jmp $ + 0x12345678
_FunctionTracePack_End:
