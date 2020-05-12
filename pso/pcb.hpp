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

	int fopen(char *fname, int mode, int type);

	int fread(int fid, ADDRESS addr, int len);

	int fwrite(int fid, ADDRESS addr, int len);

	int fseek(int fid, int pos);

	int fclose(int fid);

	int exit();

};

#endif
