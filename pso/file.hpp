#ifndef FILE_HPP
#define FILE_HPP

#include "mmu.hpp"
#include "system.hpp"

#define BUFMAX 0x200L

class XFILE {

private :
	int mode;
	int type;
	int hdl;
	char buf[BUFMAX];
	char name[40];
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

	int fopen(char *fname, int mode, int type);

	int fread(ADDRESS addr, int len);

	int fwrite(ADDRESS addr, int len);

	int fseek(int pos);

	int fclose();

};

#endif
