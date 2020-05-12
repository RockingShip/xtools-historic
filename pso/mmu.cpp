#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mmu.hpp"
#include "system.hpp"

PAGE_TABLE::PAGE_TABLE() {
	long i;

	image = (char *) malloc(65536);
	if (!image) {
		printf("out of memory");
		exit(1);
	}

	// clear image
	for (i = 0; i < 65535; i++)
		image[i] = 0;
}

PAGE_TABLE::~PAGE_TABLE() {
	free(image);
}

BOOLEAN PAGE_TABLE::read_byte(ADDRESS addr, char *value) {
	*value = image[addr & 0xFFFFL];
	return TRUE;
}

BOOLEAN PAGE_TABLE::read_word(ADDRESS addr, int *value) {
	char lo, hi;

	read_byte(addr + 0, &hi);
	read_byte(addr + 1, &lo);
	*value = (hi << 8) | (lo & 0xFF);
	return TRUE;
}

BOOLEAN PAGE_TABLE::write_byte(ADDRESS addr, char value) {
	image[addr & 0xFFFFL] = value;
	return TRUE;
}

BOOLEAN PAGE_TABLE::write_word(ADDRESS addr, int value) {
	write_byte(addr + 0, value >> 8);
	write_byte(addr + 1, value & 0xFF);
	return TRUE;
}

BOOLEAN PAGE_TABLE::loadfile(char *fname) {
	int hdl;

	if ((hdl = open(fname, O_RDONLY, 0)) == -1) {
		perror("error opening image ");
		exit(1);
	}

	read(hdl, &image[0x0000L], 0x4000);
	read(hdl, &image[0x4000L], 0x4000);
	read(hdl, &image[0x8000L], 0x4000);
	read(hdl, &image[0xC000L], 0x4000);

	close(hdl);
}
