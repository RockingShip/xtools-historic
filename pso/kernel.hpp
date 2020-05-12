#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "pcb.hpp"
#include "system.hpp"

class KERNEL {

private:
	BOOLEAN read_byte(ADDRESS addr, char *value);

	BOOLEAN read_word(ADDRESS addr, int *value);

	BOOLEAN write_byte(ADDRESS addr, char value);

	BOOLEAN write_word(ADDRESS addr, int value);

	void copystr(ADDRESS addr, char *buf);

public:
	PCB *current;

	void event(int event, int value);

	BOOLEAN supervisor(int opcode, int *R0, int *R1);

};

#endif
