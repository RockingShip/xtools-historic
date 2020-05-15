.PRECIOUS: %.xs %.xo %.xs3 %.xo3

AM_CFLAGS =  -Dint=long -fno-inline -w
EXTRA_DIST = ostest.c oscall.c
CLEANFILES = *.xs *.xo

# STAGE 1
# Native binaries

bin_PROGRAMS = xcc xasm xlnk xar

xcc_SOURCES = xcc1.c xcc2.c xcc3.c xcc.h ../getversion.c

xasm_SOURCES = xasm1.c xasm2.c xasm3.c xasm.h ../getversion.c

xlnk_SOURCES = xlnk1.c xlnk2.c xlnk.h ../getversion.c

xar_SOURCES = xar1.c xar2.c xar.h ../getversion.c

ostest.img : ostest.xo oscall.xo xlnk
	./xlnk -m ostest.map -o $@ ostest.xo oscall.xo

test-ostest : ostest.img
	../xemu ostest.img
	../pso/pso ostest.img

stage1 : xcc xasm xlnk xar
	@echo "Stage-1 complete"

# STAGE 2 *PRIMARY TARGET*
# XTools compiled with native

STAGE2_LIBS = ../lib/strcpy.xo ../lib/strncpy.xo ../lib/strlen.xo ../lib/main.xo ../lib/printf.xo ../lib/fgets.xo ../lib/fputs.xo  oscall.xo ../getversion.xo

%.xs: %.c xcc
	./xcc -S $@ $<

%.xo: %.xs xasm
	./xasm -c $@ $<

xcc.img : ./xlnk xcc1.xo xcc2.xo xcc3.xo $(STAGE2_LIBS)
	./xlnk -m xcc.map -o $@ xcc1.xo xcc2.xo xcc3.xo $(STAGE2_LIBS)

xasm.img : ./xlnk xasm1.xo xasm2.xo xasm3.xo $(STAGE2_LIBS)
	./xlnk -m xasm.map -o $@ xasm1.xo xasm2.xo xasm3.xo $(STAGE2_LIBS)

xlnk.img : ./xlnk xlnk1.xo xlnk2.xo $(STAGE2_LIBS)
	./xlnk -m xlnk.map -o $@ xlnk1.xo xlnk2.xo $(STAGE2_LIBS)

xar.img : ./xlnk xar1.xo xar2.xo $(STAGE2_LIBS)
	./xlnk -m xar.map -o $@ xar1.xo xar2.xo $(STAGE2_LIBS)

stage2 : xcc.img xasm.img xlnk.img xar.img
	@echo "Stage-2 complete"

# STAGE 3
# XTools compiled and compared with stage-2

STAGE3_LIBS = ../lib/strcpy.xo3 ../lib/strncpy.xo3 ../lib/strlen.xo3 ../lib/main.xo3 ../lib/printf.xo3 ../lib/fgets.xo3 ../lib/fputs.xo3  oscall.xo3 ../getversion.xo3

%.xs3: %.c xcc.img
	../xemu ./xcc -S $@ $<
	cmp $*.xs3 $*.xs

%.xo3: %.xs3 xasm.img
	../xemu ./xasm -c $@ $<
	cmp $*.xo3 $*.xo

stage3-xcc.img : ./xlnk xcc.img xcc1.xo3 xcc2.xo3 xcc3.xo3 $(STAGE3_LIBS) ../xemu
	./xlnk -m xcc.map -o $@ xcc1.xo3 xcc2.xo3 xcc3.xo3 $(STAGE3_LIBS)
	cmp stage3-xcc.img xcc.img

stage3-xasm.img : ./xlnk xasm.img xasm1.xo3 xasm2.xo3 xasm3.xo3 $(STAGE3_LIBS)
	./xlnk -m xasm.map -o $@ xasm1.xo3 xasm2.xo3 xasm3.xo3 $(STAGE3_LIBS)
	cmp stage3-xasm.img xasm.img

stage3-xlnk.img : ./xlnk xlnk.img xlnk1.xo3 xlnk2.xo3 $(STAGE3_LIBS)
	./xlnk -m xlnk.map -o $@ xlnk1.xo3 xlnk2.xo3 $(STAGE3_LIBS)
	cmp stage3-xlnk.img xlnk.img

stage3-xar.img : ./xlnk xar.img xar1.xo3 xar2.xo3 $(STAGE3_LIBS)
	./xlnk -m xar.map -o $@ xar1.xo3 xar2.xo3 $(STAGE3_LIBS)
	cmp stage3-xar.img xar.img

stage3 : stage3-xcc.img stage3-xasm.img stage3-xlnk.img stage3-xar.img
	@echo "Stage-3 complete"
