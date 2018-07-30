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
  sub     rsp,20h
  mov     qword [rsp+18h],rbx
  mov     qword [rsp+10h],rdx
  mov     qword [rsp+8],rcx
  push    rbp
  push    rsi
  push    rdi
  push    r12
  push    r13
  push    r14
  push    r15
  sub     rsp,60h
  mov     r12,r9
  mov     r13,r8
  mov     r10,rdx
  mov     r11,rcx

  rdtscp
  shl     rdx,20h
  or      rax,rdx
  mov     r14,rax

  mov     r15,qword [rsp+0D8h]
  mov     rbp,qword [rsp+0D0h]
  mov     rdi,qword [rsp+0C8h]
  mov     rbx,qword [rsp+0C0h]
  mov     qword [rsp+38h],r15
  mov     qword [rsp+30h],rbp
  mov     qword [rsp+28h],rdi
  mov     qword [rsp+20h],rbx
  mov     r8,r13
  mov     rdx,r10
  mov     rcx,r11
  mov rax,0DDBBCCAA44332211h ; tranmoline
  call    rax
  mov     rsi,rax

  rdtscp
  shl     rdx,20h
  or      rdx,rax
  sub     rdx,r14

  mov     qword [rsp+50h],r15
  mov     qword [rsp+48h],rbp
  mov     qword [rsp+40h],rdi
  mov     qword [rsp+38h],rbx
  mov     qword [rsp+30h],r12
  mov     qword [rsp+28h],r13
  mov     rax,qword [rsp+0A8h]  ; 2nd arg
  mov     qword [rsp+20h],rax
  mov     r9,qword [rsp+0A0h]   ; 1st arg
  lea     r8, [rsp + 0x20 + 0x38 + 0x60] ; original frame
  mov rcx,0AABBCCDD11223344h    ; pack
  mov rax, 0xffffffffffff88     ; push
  call    rax

  mov     rbx,qword [rsp+0B0h]
  mov     rax,rsi
  add     rsp,60h
  pop     r15
  pop     r14
  pop     r13
  pop     r12
  pop     rdi
  pop     rsi
  pop     rbp
  add     rsp,20h
  ret
FunctionTracePack_End:
