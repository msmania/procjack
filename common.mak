!IF "$(PLATFORM)"=="X64" || "$(PLATFORM)"=="x64"
ARCH=amd64
!ELSE
ARCH=x86
!ENDIF

OUTDIR=..\bin\$(ARCH)
OUTDIR_NOARCH=..\bin
OBJDIR=obj\$(ARCH)
OBJDIR_NOARCH=obj
SRCDIR=.
DETOUR_DIR=G:\10_Git\Detours

CC=cl
LINKER=link
NASM=D:\nasm\latest\nasm.exe
RM=del /q
RD=rd /s /q
