!include ..\common.mak

TARGET=expeb.exe
OBJS=\
    $(OBJDIR)\main.obj

LIBS=

CFLAGS=\
    /nologo\
    /Zi\
    /c\
    /Fo"$(OBJDIR)\\"\
    /Fd"$(OBJDIR)\\"\
    /DUNICODE\
    /O2\
    /EHsc\
    /W4

LFLAGS=\
    /NOLOGO\
    /DEBUG\
    /INCREMENTAL:NO\
!IF "$(PLATFORM)"=="X64"
    /SUBSYSTEM:CONSOLE,"5.02"
!ELSE
    /SUBSYSTEM:CONSOLE,"5.01"
!ENDIF

all: clean dirs $(OUTDIR)\$(TARGET)

clean:
    -@if not exist $(OBJDIR) md $(OBJDIR)
    @$(RM) /Q $(OBJDIR)\* 2>nul

dirs:
    @if not exist $(OBJDIR) mkdir $(OBJDIR)
    @if not exist $(OUTDIR) mkdir $(OUTDIR)

$(OUTDIR)\$(TARGET): $(OBJS)
    $(LINKER) $(LFLAGS) $(LIBS) /PDB:"$(@R).pdb" /OUT:$@ $**

.cpp{$(OBJDIR)}.obj:
    $(CC) $(CFLAGS) $<
