ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = LICENSE.txt CHANGELOG.md README.md .gitignore cattle.c

CLEANFILES = xlib.xa

# Directory order
SUBDIRS = pso . lib source .

# Which emulator to use. "xemu" or "pso/pso"
XEMU=xemu

bin_PROGRAMS = xemu

xemu_SOURCES = xemu.c getversion.c

all-local:
	@echo 
	@echo "stage 1 has been built"
	@echo "xcc does not support the -I include path"
	@echo "run \"make stage2\" to build XTools/XLibrary in this directory"
	@echo "run \"make stage3\" to verify stage 2 can build itself and produce identical output"
	@echo 

stage2-local:
	@echo 
	@echo "The following files should be present:"
	@echo "  xcc.img   X-C-Compiler"
	@echo "  xasm.img  X-Asselmber"
	@echo "  xlnk.img  X-Linker"
	@echo "  xar.img   X-Archiver"
	@echo "  xlib.xa   X-Library to be used with \"xlnk ... -l xlib\""
	@echo "  xemu      Native emulator"
	@echo 
	@echo "Try:"
	@echo " ./xemu xcc  cattle.c"
	@echo " ./xemu xasm cattle"
	@echo " ./xemu xlnk cattle -l xlib"
	@echo " ./xemu cattle"
	@echo 
