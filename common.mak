!IF "$(PLATFORM)"=="X64" || "$(PLATFORM)"=="x64"
ARCH=amd64
!ELSE
ARCH=x86
!ENDIF

OUTDIR=..\bin\$(ARCH)
OBJDIR=obj\$(ARCH)
SRCDIR=.

CC=cl
LINKER=link
NASM=D:\nasm\latest\nasm.exe
RM=del /q
RD=rd /s /q
