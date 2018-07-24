BITS 64

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
