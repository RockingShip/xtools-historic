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

int PCB::fopen(char *fname, int mode, int type) {
	int fid;

	// locate free slot
	for (fid = 6; fid < FILEMAX; fid++)
		if (files[fid] == NULL)
			break;

	if ((fid < 0) || (fid > FILEMAX))
		return -1;
	files[fid] = new XFILE(&pagetable);
	files[fid]->fopen(fname, mode, type);
	return fid;
}

int PCB::fread(int fid, ADDRESS addr, int len) {
	if ((fid < 0) || (fid > FILEMAX) || (files[fid] == NULL))
		return -1;
	return files[fid]->fread(addr, len);
}

int PCB::fwrite(int fid, ADDRESS addr, int len) {
	if ((fid < 0) || (fid > FILEMAX) || (files[fid] == NULL))
		return -1;
	return files[fid]->fwrite(addr, len);
}

int PCB::fseek(int fid, int pos) {
	if ((fid < 0) || (fid > FILEMAX) || (files[fid] == NULL))
		return -1;
	return files[fid]->fseek(pos);
}

int PCB::fclose(int fid) {
	if ((fid < 0) || (fid > FILEMAX) || (files[fid] == NULL))
		return -1;
	return files[fid]->fclose();
}

int PCB::exit() {
	int i;

	for (i = 0; i < FILEMAX; i++)
		if (files[i] != NULL)
			fclose(i);
}
