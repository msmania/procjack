!include ..\common.mak

TARGET=spy.dll
DEF=spy.def
OBJS=\
	$(OBJDIR)\dllmain.obj\
	$(OBJDIR)\spy.res\

# warning C4100: unreferenced formal parameter
CFLAGS=\
	/nologo\
	/Zi\
	/c\
	/Fo"$(OBJDIR)\\"\
	/Fd"$(OBJDIR)\\"\
	/DUNICODE\
	/Od\
	/EHsc\
	/W4\
	/wd4100\

LFLAGS=\
	/NOLOGO\
	/DEBUG\
	/SUBSYSTEM:WINDOWS\
	/DLL\
	/DEF:$(DEF)\

LIBS=\

all: $(OUTDIR)\$(TARGET)

$(OUTDIR)\$(TARGET): $(OBJS)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
	$(LINKER) $(LFLAGS) $(LIBS) /PDB:"$(@R).pdb" /OUT:$@ $**

{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	$(CC) $(CFLAGS) $<

{$(SRCDIR)}.rc{$(OBJDIR)}.res:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	rc /nologo /fo "$@" $<

clean:
	@if exist $(OBJDIR) $(RD) $(OBJDIR)
	@if exist $(OUTDIR)\$(TARGET) $(RM) $(OUTDIR)\$(TARGET)
	@if exist $(OUTDIR)\$(TARGET:dll=ilk) $(RM) $(OUTDIR)\$(TARGET:dll=ilk)
	@if exist $(OUTDIR)\$(TARGET:dll=pdb) $(RM) $(OUTDIR)\$(TARGET:dll=pdb)
	@if exist $(OUTDIR)\$(TARGET:dll=exp) $(RM) $(OUTDIR)\$(TARGET:dll=exp)
	@if exist $(OUTDIR)\$(TARGET:dll=lib) $(RM) $(OUTDIR)\$(TARGET:dll=lib)
