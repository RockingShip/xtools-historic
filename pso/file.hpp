#ifndef FILE_HPP
#define FILE_HPP

#include "mmu.hpp"
#include "system.hpp"

#define BUFMAX  0x200L
#define PATHMAX 80 /* Length of filename */

class XFILE {

private :
	int mode;
	int type;
	int hdl;
	char buf[BUFMAX];
	char name[PATHMAX];
	long bufpos;
	long filpos;
	int buflen;
	int bufofs;
	int dirty;
	PAGE_TABLE *pagetable;

	int fload();

	int fflush();

public :
	XFILE(PAGE_TABLE *pagetable);

	int fopen(char *fname, char *mode);

	int fread(ADDRESS addr, int siz, int cnt);

	int fwrite(ADDRESS addr, int siz, int cnt);

	int fseek(int pos, int whence);

	int ftell();

	int fclose();

};

#endif
