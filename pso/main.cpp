#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.hpp"
#include "kernel.hpp"
#include "mmu.hpp"
#include "system.hpp"

KERNEL kernel;
CPU cpu;

char *fext(char *path, const char *ext) {
	char *p;
	int  baselen;

	baselen = 0;
	for (p  = path; *p; p++) {
		if (*p == '\\' || *p == '/')
			baselen = 0;
		else if (*p == '.')
			baselen = p - path;
	}

	if (!baselen)
		return path;

	p = (char *) malloc(baselen + strlen(ext) + 1);
	strncpy(p, path, baselen);
	strcpy(p + baselen, ext);

	return p;
}

int main(int argc, char *argv[]) {
	int i;

	setlinebuf(stdout);

	kernel.current = new PCB();
	// generate complete commandline
	for (i = 2; i < argc; i++) {
		if (i != 2)
			strcat(kernel.current->command, " ");
		strcat(kernel.current->command, argv[i]);
	}
	cpu.load_context(&kernel.current->context, &kernel.current->pagetable);

	printf("loading...\n");
	cpu.loadfile(fext(argv[1], ".img"));

	printf("starting...\n");
	while (1)
		cpu.tick();

	return 0;
}
