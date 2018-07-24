BITS 32

section .text

Measure_Start:
  push eax
  push edx
  push esi

  mov eax, 0x7ffffffd   ; CallCount
  inc dword [eax]

  mov esi, 0x7fffffff   ; tick_start_
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
  mov     dword [esp+20h], 0x7ffffffe ; total_ticks_
  mov     ecx, 0x7fffffff             ; tick_start_
  sub     ebp,dword [ecx]
  mov     eax,edx
  sbb     eax,dword [ecx+0x4]
  mov     dword [esp+24h],ebp
  mov     dword [esp+1Ch],eax

label_0x30:
  mov     ecx, 0x7ffffffe             ; total_ticks_
  mov     edi,dword [ecx]
  mov     ebx,edi
  mov     esi,dword [ecx+0x4]
  add     ebx,ebp
  mov     ecx,esi
  mov     edx,esi
  adc     ecx,eax
  mov     eax,edi
  mov     ebp,dword [esp+20h]         ; total_ticks_
  lock cmpxchg8b [ebp]
  mov     ebp,dword [esp+24h]
  cmp     eax,edi
  mov     eax,dword [esp+1Ch]
  jne     label_0x30

  cmp     edx,esi
  jne     label_0x30

  mov eax, 0x7ffffffd   ; CallCount
  inc dword [eax]

  pop     edi
  pop     esi
  pop     ebp
  pop     edx
  pop     ecx
  pop     ebx
  pop     eax
  add     esp,0Ch
  jmp $ + 0x12345678

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
