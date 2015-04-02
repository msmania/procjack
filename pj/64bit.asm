[org 0]
[bits 64]

mov     qword [rsp+8],rbx
push    rdi
sub     rsp,30h
mov     rbx,rcx
add     rcx,400h
mov     rax, 0xffffffff12345678
call    rax ; LoadLibrary
mov     edx,0DEADh
mov     rcx,rax
mov     rdi,rax
mov     rax, 0xffffffff12345678
call    rax ; GetProcAddress
xor     ecx,ecx
mov     r9,rbx
mov     qword [rsp+28h],rcx
mov     r8,rax
xor     edx,edx
mov     dword [rsp+20h],ecx
mov     rax, 0xffffffff12345678
call    rax ; CreateThread
or      edx,0FFFFFFFFh
mov     rcx,rax
mov     rbx,rax
mov     rax, 0xffffffff12345678
call    rax ; WaitForSingleObject
mov     rcx,rbx
mov     rax, 0xffffffff12345678
call    rax ; CloseHandle
mov     rcx,rdi
mov     rax, 0xffffffff12345678
call    rax ; FreeLibrary
xor     ecx,ecx
mov     rax, 0xffffffff12345678
call    rax ; ExitThread
int     3
int     3
