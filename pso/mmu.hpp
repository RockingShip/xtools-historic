#ifndef MMU_HPP
#define MMU_HPP

#include "system.hpp"

class PAGE_TABLE {

private:
	char *image;

public:
	PAGE_TABLE();

	~PAGE_TABLE();

	BOOLEAN read_byte(ADDRESS addr, char *value);

	BOOLEAN read_word(ADDRESS addr, int *value);

	BOOLEAN write_byte(ADDRESS addr, char value);

	BOOLEAN write_word(ADDRESS addr, int value);

	BOOLEAN loadfile(char *fname);

};

#endif
