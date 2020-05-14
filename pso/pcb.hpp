#ifndef PCB_HPP
#define PCB_HPP

#include "context.hpp"
#include "file.hpp"
#include "mmu.hpp"
#include "system.hpp"

class PCB {

private :

#define FILEMAX 16

	XFILE *files[FILEMAX];

public :

	CPU_CONTEXT context;

	PAGE_TABLE pagetable;

	char command[128];

	PCB();

	int fopen(char *fname, char *mode);

	int fread(ADDRESS addr, int siz, int cnt, int hdl);

	int fwrite(ADDRESS addr, int siz, int cnt, int hdl);

	int fseek(int fid, int pos, int whence);

	int ftell(int fid);

	int fclose(int fid);

	int exit();

};

#endif
