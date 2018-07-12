BITS 64

extern Counter_Total
extern InjectionPoint_Start
extern InjectionPoint_End

global Clove_Start
global Clove_End

section .data
Counter_Local: dq 0

section .text

Clove_Start:
  push rdx
  rdtsc
  shl rdx,20h
  or  rax,rdx
  mov rdx, Counter_Local
  mov [rdx], rax
  pop rdx

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

  mov rax, InjectionPoint_End
  jmp [rax]
