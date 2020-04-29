!include ..\common.mak

TARGET=pj.exe
OBJS=\
	$(OBJDIR)\main.obj\
	$(OBJDIR)\auth_utils.obj\
	$(OBJDIR)\pkg_creator.obj\
	$(OBJDIR)\utils.obj\
	$(OBJDIR)\policy.obj\

LIBS=\
	advapi32.lib\
	authz.lib\
	user32.lib\

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
	/DDOWNLEVEL\

LFLAGS=\
	/NOLOGO\
	/DEBUG\
	/SUBSYSTEM:CONSOLE\

all: $(OUTDIR)\$(TARGET)

$(OUTDIR)\$(TARGET): $(OBJS)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
	$(LINKER) $(LFLAGS) $(LIBS) /PDB:"$(@R).pdb" /OUT:$@ $**

{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	$(CC) $(CFLAGS) $<

clean:
	@if exist $(OBJDIR) $(RD) $(OBJDIR)
	@if exist $(OUTDIR)\$(TARGET) $(RM) $(OUTDIR)\$(TARGET)
	@if exist $(OUTDIR)\$(TARGET:exe=ilk) $(RM) $(OUTDIR)\$(TARGET:exe=ilk)
	@if exist $(OUTDIR)\$(TARGET:exe=pdb) $(RM) $(OUTDIR)\$(TARGET:exe=pdb)
