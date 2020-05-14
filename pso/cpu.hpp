#ifndef CPU_HPP
#define CPU_HPP

#include "alu.hpp"
#include "context.hpp"
#include "mmu.hpp"
#include "system.hpp"

class CPU {

private :
	CPU_CONTEXT context;
	PAGE_TABLE *pagetable;
	ALU alu;
	int startpc;

	BOOLEAN read_byte(ADDRESS addr, char *value);

	BOOLEAN read_word(ADDRESS addr, int *value);

	BOOLEAN write_byte(ADDRESS addr, char value);

	BOOLEAN write_word(ADDRESS addr, int value);

	BOOLEAN get_pc_byte(char *value);

	BOOLEAN get_pc_word(int *value);

	BOOLEAN get_ea(int *value);

public :
	void tick();

	void load_context(CPU_CONTEXT *context, PAGE_TABLE *pagetable);

	void save_context(CPU_CONTEXT *context);

	BOOLEAN loadfile(char *fname);

	void pushArgs(char *argv[]);
};

#endif
