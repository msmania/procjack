BITS 64

section .data
Counter_Local dq 0
Counter_Total dq 0
InjectionPoint_Start dq 0
InjectionPoint_End dq 0

section .text

Measure_Start:
  push rax
  push rdx
  rdtsc
  shl rdx,20h
  or  rax,rdx
  mov rdx, Counter_Local
  mov [rdx], rax
  pop rdx
  pop rax

  jmp $ + 0x12345678
  ;mov rax, InjectionPoint_Start
  ;jmp [rax]

Measure_End:
  push rax
  push rdx
  rdtsc
  shl rdx,20h
  or  rax,rdx
  mov rdx, Counter_Local
  sub rax, [rdx]
  mov rdx, Counter_Total
  lock xadd qword [rdx], rax
  pop rdx
  pop rax

  jmp $ + 0x12345678
  ;mov rax, InjectionPoint_End
  ;jmp [rax]

FunctionCallPack_Start:
  push rax
  push rcx
  push rdx

  sub rsp, 10h
  mov rax, [rsp + 28h] ; return address
  mov [rsp], rax
  mov [rsp + 8], rcx   ; 1st argument

  mov rdx, rsp
  mov rcx, 0x7ffffffffffffff  ; FunctionCallPack instance
  mov rax, 0x7fffffffffffffe  ; EntryPoint
  call rax

  add rsp, 10h

  pop rdx
  pop rcx
  pop rax
  jmp $ + 0x12345678
