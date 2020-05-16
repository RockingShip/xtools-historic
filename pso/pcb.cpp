#include <stdio.h>
#include <stdlib.h>
#include "pcb.hpp"
#include "system.hpp"

PCB::PCB() {
	int i;

	for (i = 0; i < FILEMAX; i++)
		files[i] = NULL;

	files[0] = new XFILE(&pagetable);
	files[0]->fdopen(0, "r");
	files[1] = new XFILE(&pagetable);
	files[1]->fdopen(1, "w");
	files[2] = new XFILE(&pagetable);
	files[2]->fdopen(2, "w");

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
	if (files[fid]->fopen(fname, mode) == -1) {
		/* open failed */
		delete files[fid];
		files[fid] = NULL;
		return 0;
	}
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
	delete files[fid];
	files[fid] = NULL;

	return ret;
}

int PCB::exit() {
	int i;

	for (i = 0; i < FILEMAX; i++)
		if (files[i] != NULL)
			fclose(i);
}
