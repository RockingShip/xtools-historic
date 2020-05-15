x.PRECIOUS: %.xs

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

xcc.img : xcc1.xo xcc2.xo xcc3.xo ../lib/strcpy.xo ../lib/strncpy.xo ../lib/strlen.xo ../lib/main.xo ../lib/printf.xo ../lib/fgets.xo ../lib/fputs.xo oscall.xo xlnk
	./xlnk -m xcc.map -o $@ xcc1.xo xcc2.xo xcc3.xo ../lib/strcpy.xo ../lib/strncpy.xo ../lib/strlen.xo ../lib/main.xo ../lib/printf.xo ../lib/fgets.xo ../lib/fputs.xo oscall.xo 

test-ostest : ostest.img
	../xemu ostest.img
	../pso/pso ostest.img
