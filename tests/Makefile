!include ..\common.mak

SRCDIR=src

TARGET=t.exe

OBJS=\
	$(OBJDIR)\main.obj\
	$(OBJDIR)\utils_test.obj\
	..\clove\obj\$(ARCH)\page.obj\
	..\clove\obj\$(ARCH)\utils.obj\
	..\pj\obj\$(ARCH)\utils.obj\

LIBS=\
	gmock_main.lib\
	detours.lib\

CFLAGS=\
	/nologo\
	/c\
	/Od\
	/W4\
	/Zi\
	/EHsc\
	/DUNICODE\
	/D_CRT_SECURE_NO_WARNINGS\
	/Fo"$(OBJDIR)\\"\
	/Fd"$(OBJDIR)\\"\
	/I"$(GTEST_SRC_DIR)\googletest\include"\
	/I"$(GTEST_SRC_DIR)\googlemock\include"\

LFLAGS=\
	/NOLOGO\
	/DEBUG\
	/SUBSYSTEM:CONSOLE\
	/LIBPATH:"$(GTEST_BUILD_DIR)\googlemock\Release"\
	/LIBPATH:"$(GTEST_BUILD_DIR)\googlemock\gtest\Release"\
	/LIBPATH:"$(DETOUR_DIR)\lib.$(PLATFORM)"\

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
