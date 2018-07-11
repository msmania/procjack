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
	/O2\
	/W4\

AFLAGS=\

LFLAGS=\
	/NOLOGO\
	/DEBUG\
!IF "$(PLATFORM)"=="X64" || "$(PLATFORM)"=="x64"
	/SUBSYSTEM:CONSOLE,"5.02"\
!ELSE
	/SUBSYSTEM:CONSOLE,"5.01"\
!ENDIF

all: $(OUTDIR)\$(TARGET) $(OUTDIR_NOARCH)\expeb64.bin $(OUTDIR_NOARCH)\expeb32.bin

$(OUTDIR)\$(TARGET): $(OBJS)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
	$(LINKER) $(LFLAGS) $(LIBS) /PDB:"$(@R).pdb" /OUT:$@ $**

{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	$(CC) $(CFLAGS) $<

$(OUTDIR_NOARCH)\expeb64.bin: $(SRCDIR)\expeb64.asm
	$(NASM) $(AFLAGS) -o $@ -l $(OBJDIR_NOARCH)\$(@B).lst $**

$(OUTDIR_NOARCH)\expeb32.bin: $(SRCDIR)\expeb32.asm
	$(NASM) $(AFLAGS) -o $@ -l $(OBJDIR_NOARCH)\$(@B).lst $**

clean:
	@if exist $(OBJDIR) $(RD) $(OBJDIR)
	@if exist $(OUTDIR)\$(TARGET) $(RM) $(OUTDIR)\$(TARGET)
	@$(RM) $(OBJDIR_NOARCH)\expeb*.lst
	@if exist $(OUTDIR_NOARCH)\expeb64.bin $(RM) $(OUTDIR_NOARCH)\expeb64.bin
	@if exist $(OUTDIR_NOARCH)\expeb32.bin $(RM) $(OUTDIR_NOARCH)\expeb32.bin
	@if exist $(OUTDIR)\$(TARGET:exe=ilk) $(RM) $(OUTDIR)\$(TARGET:exe=ilk)
	@if exist $(OUTDIR)\$(TARGET:exe=pdb) $(RM) $(OUTDIR)\$(TARGET:exe=pdb)
