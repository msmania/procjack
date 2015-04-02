!IF "$(PLATFORM)"=="X64"
OUTDIR=..\bin64
OBJDIR=obj64
!ELSE
OUTDIR=..\bin
OBJDIR=obj
!ENDIF

CC=cl
LINKER=link
RM=del /q
