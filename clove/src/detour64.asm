BITS 64

global MeasurementChain_Start
global MeasurementChain_Checkpoint
global MeasurementChain_End
global FunctionTracePack_Start
global FunctionTracePack_End

section .text

MeasurementChain_Start:
  push rax
  push rdx
  mov     rax, 08FFFFFFFFFFFFFF0h   ; callcount
  inc     dword [rax]
  rdtsc
  shl     rdx,20h
  or      rax,rdx
  mov     rdx, 08FFFFFFFFFFFFFF8h   ; checkpoint
  mov     qword [rdx],rax
  pop rdx
  pop rax
  jmp $ + 0x12345678

MeasurementChain_Checkpoint:
  push rax
  push rcx
  push rdx
  mov rax, 08FFFFFFFFFFFFFE0h       ; total
  mov     rcx,qword [rax]
  mov rax, 08FFFFFFFFFFFFFF8h       ; checkpoint
  sub     rcx,qword [rax]
  rdtsc
  shl     rdx,20h
  or      rax,rdx
  add     rcx,rax
  mov     rdx, 08FFFFFFFFFFFFFE0h   ; total
  mov     qword [rdx],rcx
  mov     rdx, 08FFFFFFFFFFFFFE8h   ; checkpoint
  mov     qword [rdx],rax

  mov     rax, 08FFFFFFFFFFFFFF0h   ; callcount
  inc     dword [rax]
  pop rdx
  pop rcx
  pop rax
  jmp $ + 0x12345678
MeasurementChain_End:

FunctionTracePack_Start:
  sub rsp, 88h
  mov [rsp+00h], rax
  mov [rsp+08h], rbx
  mov [rsp+10h], rcx
  mov [rsp+18h], rdx
  mov [rsp+20h], rsi
  mov [rsp+28h], rdi
  lea rax, [rsp+88h]
  mov [rsp+30h], rax
  mov [rsp+38h], rbp
  mov [rsp+40h], r8
  mov [rsp+48h], r9
  mov [rsp+50h], r10
  mov [rsp+58h], r11
  mov [rsp+60h], r12
  mov [rsp+68h], r13
  mov [rsp+70h], r14
  mov [rsp+78h], r15

  mov rdx, rsp
  mov rcx, 0xAABBCCDD11223344 ; this
  mov rax, 0xffffffffffff88aa ; func
  sub rsp, 20h
  call rax
  add rsp, 20h

  mov rax, [rsp+00h]
  mov rbx, [rsp+08h]
  mov rcx, [rsp+10h]
  mov rdx, [rsp+18h]
  mov rsi, [rsp+20h]
  mov rdi, [rsp+28h]
  mov rbp, [rsp+38h]
  mov r8,  [rsp+40h]
  mov r9,  [rsp+48h]
  mov r10, [rsp+50h]
  mov r11, [rsp+58h]
  mov r12, [rsp+60h]
  mov r13, [rsp+68h]
  mov r14, [rsp+70h]
  mov r15, [rsp+78h]
  mov rsp, [rsp+30h]

  jmp $ + 0x12345678
FunctionTracePack_End:
