!include ..\common.mak

TARGET=pj.exe
OBJS=\
    $(OBJDIR)\main.obj

LIBS=advapi32.lib\
     authz.lib

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
    /SUBSYSTEM:CONSOLE

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
