.PRECIOUS: %.xs %.xo %.xs3 %.xo3

EXTRA_DIST = $(MODULES)

# Which emulator to use. "xemu" or "pso/pso"
XEMU=xemu

CLEANFILES = *.xs *.xo *.xs3 *.xo3 stamp-stage2 stamp-stage3 stage3-xlib.xa

MODULES = \
	fgets.c \
	fputs.c \
	isalnum.c \
	isalpha.c \
	iscntrl.c \
	isdigit.c \
	isgraph.c \
	islower.c \
	isprint.c \
	ispunct.c \
	isspace.c \
	isupper.c \
	isxdigit.c \
	main.c \
	oscall.c \
	printf.c \
	puts.c \
	reverse.c \
	scanf.c \
	strcat.c \
	strchr.c \
	strcmp.c \
	strcpy.c \
	strlen.c \
	strncat.c \
	strncmp.c \
	strncpy.c \
	strrchr.c \
	tolower.c \
	toupper.c

# STAGE 2 *PRIMARY TARGET*
# XLibrary compiled and constructed with native

%.xs: %.c
	../source/xcc -S $@ $<

%.xo: %.xs
	../source/xasm -c $@ $<
	../source/xar a ../xlib.xa $@

stamp-stage2: $(subst .c,.xo,$(MODULES))
	touch $@

stage2-local: stamp-stage2
	@echo "Stage-2 XLibrary complete"

# STAGE 3 
# XLibrary compiled, constructed and compares with stage-2

%.xs3: %.c
	../$(XEMU) ../xcc -S $@ $<
	cmp $*.xs3 $*.xs

%.xo3: %.xs3
	../$(XEMU) ../xasm -c $@ $<
	cmp $*.xo3 $*.xo
	../$(XEMU) ../xar a stage3-xlib.xa $@
	../xemu ../xar a s2.xa $@
	../pso/pso ../xar a s3.xa $@
	cmp s2.xa s3.xa

stamp-stage3: $(subst .c,.xo3,$(MODULES))
	cmp stage3-xlib.xa ../xlib.xa
	touch $@

stage3-local: stamp-stage3
	@echo "Stage-3 XLibrary complete"

stage3-clean :
	rm *.xs3 *.xo3 stage3-xlib.xa
