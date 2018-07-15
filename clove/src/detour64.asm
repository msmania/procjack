BITS 64

global Clove_Start
global Clove_End

section .data
Counter_Local dq 0
Counter_Total dq 0
InjectionPoint_Start dq 0
InjectionPoint_End dq 0

section .text

Clove_Start:
  push rdx
  rdtsc
  shl rdx,20h
  or  rax,rdx
  mov rdx, Counter_Local
  mov [rdx], rax
  pop rdx

  jmp $ + 0x12345678
  mov rax, InjectionPoint_Start
  jmp [rax]

Clove_End:
  push rdx
  rdtsc
  shl rdx,20h
  or  rax,rdx
  mov rdx, Counter_Local
  sub rax, [rdx]
  mov rdx, Counter_Total
  lock xadd qword [rdx], rax
  pop rdx

  jmp $ + 0x12345678
  mov rax, InjectionPoint_End
  jmp [rax]
