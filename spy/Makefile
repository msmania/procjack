!include ..\common.mak

TARGET=spy.dll
DEF=spy.def
OBJS=\
    $(OBJDIR)\dllmain.obj

# warning C4100: unreferenced formal parameter
CFLAGS=\
    /nologo\
    /Zi\
    /c\
    /Fo"$(OBJDIR)\\"\
    /Fd"$(OBJDIR)\\"\
    /DUNICODE\
    /O2\
    /EHsc\
    /W4\
    /wd4100

LFLAGS=\
    /NOLOGO\
    /DEBUG\
    /SUBSYSTEM:WINDOWS\
    /DLL\
    /DEF:$(DEF)\
    /INCREMENTAL:NO

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
