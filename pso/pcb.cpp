#include <stdio.h>
#include <stdlib.h>
#include "pcb.hpp"
#include "system.hpp"

PCB::PCB() {
	int i;

	for (i = 0; i < FILEMAX; i++)
		files[i] = NULL;
	command[0] = 0;
}

int PCB::fopen(char *fname, char *mode) {
	int fid;

	// locate free slot
	for (fid = 6; fid < FILEMAX; fid++)
		if (files[fid] == NULL)
			break;

	if ((fid < 0) || (fid >= FILEMAX))
		return -1;
	files[fid] = new XFILE(&pagetable);
	files[fid]->fopen(fname, mode);
	return fid;
}

int PCB::fread(ADDRESS addr, int siz, int cnt, int fid) {
	if ((fid < 0) || (fid >= FILEMAX) || (files[fid] == NULL))
		return -1;
	return files[fid]->fread(addr, siz, cnt);
}

int PCB::fwrite(ADDRESS addr, int siz, int cnt, int fid) {
	if ((fid < 0) || (fid >= FILEMAX) || (files[fid] == NULL))
		return -1;
	return files[fid]->fwrite(addr, siz, cnt);
}

int PCB::fseek(int fid, int pos, int whence) {
	if ((fid < 0) || (fid >= FILEMAX) || (files[fid] == NULL))
		return -1;
	return files[fid]->fseek(pos, whence);
}

int PCB::ftell(int fid) {
	if ((fid < 0) || (fid >= FILEMAX) || (files[fid] == NULL))
		return -1;
	return files[fid]->ftell();
}

int PCB::fclose(int fid) {
	if ((fid < 0) || (fid >= FILEMAX) || (files[fid] == NULL))
		return -1;

	int ret = files[fid]->fclose();
	// release handle
	files[fid] = NULL;

	return ret;
}

int PCB::exit() {
	int i;

	for (i = 0; i < FILEMAX; i++)
		if (files[i] != NULL)
			fclose(i);
}
