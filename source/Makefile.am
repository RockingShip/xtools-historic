.PRECIOUS: %.xs %.xo %.xs3 %.xo3

AM_CFLAGS =  -Dint=long -fno-inline -w
EXTRA_DIST = ostest.c
CLEANFILES = *.xs *.xo *.xs3 *.xo3 *.map

# Which emulator to use. "xemu" or "pso/pso"
XEMU=xemu

# STAGE 1
# Native binaries

bin_PROGRAMS = xcc xasm xlnk xar

xcc_SOURCES = xcc1.c xcc2.c xcc3.c xcc.h getversion.c

xasm_SOURCES = xasm1.c xasm2.c xasm3.c xasm.h getversion.c

xlnk_SOURCES = xlnk1.c xlnk2.c xlnk.h getversion.c

xar_SOURCES = xar1.c xar2.c xar.h getversion.c

ostest.img : ostest.xo ../lib/main.xo xlnk
	./xlnk -m ostest.map -o $@ ostest.xo ../lib/main.xo

test-ostest : ostest.img
	../xemu ostest.img
	../pso/pso ostest.img

stage1 : xcc xasm xlnk xar
	@echo "Stage-1 XTools complete"

# STAGE 2 *PRIMARY TARGET*
# XTools compiled with native

%.xs: %.c
	./xcc -S $@ $<

%.xo: %.xs
	./xasm -c $@ $<

../xcc.img : xcc1.xo xcc2.xo xcc3.xo getversion.xo
	./xlnk -m xcc.map -o $@ $^ -l ../xlib.xa

../xasm.img : xasm1.xo xasm2.xo xasm3.xo getversion.xo
	./xlnk -m xasm.map -o $@ $^ -l ../xlib.xa

../xlnk.img : xlnk1.xo xlnk2.xo getversion.xo
	./xlnk -m xlnk.map -o $@ $^ -l ../xlib.xa

../xar.img : xar1.xo xar2.xo getversion.xo
	./xlnk -m xar.map -o $@ $^ -l ../xlib.xa

stage2 : ../xcc.img ../xasm.img ../xlnk.img ../xar.img
	@echo "Stage-2 XTools complete"

# STAGE 3
# XTools compiled and compared with stage-2

%.xs3: %.c
	../$(XEMU) ../xcc -S $@ $<
	cmp $*.xs3 $*.xs

%.xo3: %.xs3
	../$(XEMU) ../xasm -c $@ $<
	cmp $*.xo3 $*.xo

# @date 2020-06-07 20:57:12
# need .xo3 to trigger build rules but need .xo to link because ".xo3" results in a different hashtable.
# This linker uses the hasttable to import library modules, ".xo3" might result in a different yet correct image.
stage3-xcc.img : xcc1.xo3 xcc2.xo3 xcc3.xo3 getversion.xo3
	../$(XEMU) ../xlnk -m stage3-xcc.map -o $@ xcc1.xo xcc2.xo xcc3.xo getversion.xo -l ../xlib.xa
	cmp stage3-xcc.img ../xcc.img

stage3-xasm.img : xasm1.xo3 xasm2.xo3 xasm3.xo3 getversion.xo3
	../$(XEMU) ../xlnk -m stage3-xasm.map -o $@ xasm1.xo xasm2.xo xasm3.xo getversion.xo -l ../xlib.xa
	cmp stage3-xasm.img ../xasm.img

stage3-xlnk.img : xlnk1.xo3 xlnk2.xo3 getversion.xo3
	../$(XEMU) ../xlnk -m stage3-xlnk.map -o $@ xlnk1.xo xlnk2.xo getversion.xo -l ../xlib.xa
	cmp stage3-xlnk.img ../xlnk.img

stage3-xar.img : xar1.xo3 xar2.xo3 getversion.xo3
	../$(XEMU) ../xlnk -m stage3-xar.map -o $@ xar1.xo xar2.xo getversion.xo -l ../xlib.xa
	cmp stage3-xar.img ../xar.img

stage3 : stage3-xcc.img stage3-xasm.img stage3-xlnk.img stage3-xar.img
	@echo "Stage-3 XTools complete"
