!include ..\common.mak

SRCDIR=src

CC=cl
ASM=D:\nasm\latest\nasm.exe
LINKER=link
RM=del/q
RD=rd/s/q

TARGET=clove.dll
DEF=$(SRCDIR)\clove.def
OBJS=\
	$(OBJDIR)\dllmain.obj\
	$(OBJDIR)\detour.obj\
!IF "$(ARCH)"=="amd64"
	$(OBJDIR)\detour64.obj\
!ENDIF
	$(OBJDIR)\event.obj\
	$(OBJDIR)\measurement.obj\
	$(OBJDIR)\page.obj\
	$(OBJDIR)\utils.obj\

LIBS=\
	user32.lib\
	detours.lib\

CFLAGS=\
	/nologo\
	/c\
	/Zi\
	/DUNICODE\
	/O2\
	/W4\
	/EHsc\
	/Fo"$(OBJDIR)\\"\
	/Fd"$(OBJDIR)\\"\
	/I"$(DETOUR_DIR)\include"\

AFLAGS=\
!IF "$(ARCH)"=="amd64"
	-fwin64\
!ELSE
	-fwin32\
!ENDIF
	-g\
	-O0\

LFLAGS=\
	/NOLOGO\
	/DEBUG\
	/SUBSYSTEM:WINDOWS\
	/DLL\
	/DEF:$(DEF)\
	/LIBPATH:"$(DETOUR_DIR)\lib.$(PLATFORM)"\

all: $(OUTDIR)\$(TARGET)

$(OUTDIR)\$(TARGET): $(OBJS)
	@if not exist $(OUTDIR) mkdir $(OUTDIR)
	$(LINKER) $(LFLAGS) $(LIBS) /PDB:"$(@R).pdb" /OUT:$@ $**

{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	$(CC) $(CFLAGS) $<

{$(SRCDIR)}.asm{$(OBJDIR)}.obj:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	$(NASM) $(AFLAGS) -l $(@R).lst -o $@ $<

clean:
	@if exist $(OBJDIR) $(RD) $(OBJDIR)
	@if exist $(OUTDIR)\$(TARGET) $(RM) $(OUTDIR)\$(TARGET)
	@if exist $(OUTDIR)\$(TARGET:dll=ilk) $(RM) $(OUTDIR)\$(TARGET:dll=ilk)
	@if exist $(OUTDIR)\$(TARGET:dll=pdb) $(RM) $(OUTDIR)\$(TARGET:dll=pdb)
	@if exist $(OUTDIR)\$(TARGET:dll=exp) $(RM) $(OUTDIR)\$(TARGET:dll=exp)
	@if exist $(OUTDIR)\$(TARGET:dll=lib) $(RM) $(OUTDIR)\$(TARGET:dll=lib)
