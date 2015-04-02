[org 0]
[bits 32]

push    esi
mov     esi,dword [esp+8]
push    edi
lea     eax,[esi+400h]
push    eax
mov     eax, 0x12345678
call    eax ; LoadLibraryW
mov     edi,eax
push    0DEADh
push    edi
mov     eax, 0x12345678
call    eax ; GetProcAddress
push    0
push    0
push    esi
push    eax
push    0
push    0
mov     eax, 0x12345678
call    eax ; CreateThread
mov     esi,eax
push    0FFFFFFFFh
push    esi
mov     eax, 0x12345678
call    eax ; WaitForSingleObject
push    esi
mov     eax, 0x12345678
call    eax ; CloseHandle
push    edi
mov     eax, 0x12345678
call    eax ; FreeLibrary
push    0
mov     eax, 0x12345678
call    eax ; ExitThread
pop     edi
pop     esi
int     3
int     3
