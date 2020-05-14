AM_CFLAGS =  -Dint=long -fno-inline -w
EXTRA_DIST = ostest.c oscall.c

bin_PROGRAMS = xcc xasm xlnk xar

xcc_SOURCES = xcc1.c xcc2.c xcc3.c xcc.h

xasm_SOURCES = xasm1.c xasm2.c xasm3.c xasm.h

xlnk_SOURCES = xlnk1.c xlnk2.c xlnk.h

xar_SOURCES = xar1.c xar2.c xar.h

%.xs : %.c xcc
	./xcc -S $@ $<

%.xo : %.xs xasm
	./xasm -c $@ $<

ostest.img : ostest.xo oscall.xo xlnk
	./xlnk -m ostest.map -o $@ ostest.xo oscall.xo

xcc.img : xcc1.xo xcc2.xo xcc3.xo xlnk
	./xlnk -m xcc.map -o $@ xcc1.xo xcc2.xo xcc3.xo

test-ostest : ostest.img
	../xemu ostest.img
	../pso/pso ostest.img
