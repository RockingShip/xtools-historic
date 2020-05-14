#include <stdio.h>
#include <string.h>
#include "cpu.hpp"
#include "kernel.hpp"
#include "opcodes.hpp"
#include "system.hpp"

// KERNEL is a global object
extern KERNEL kernel;

BOOLEAN CPU::read_byte(ADDRESS addr, char *value) {
	// read memory location
	if (!pagetable->read_byte(addr, value)) {
		// call kernel with error status
		context.setpc(startpc);
		kernel.event(1, addr);
		return FALSE;
	}
	return TRUE;
}

BOOLEAN CPU::read_word(ADDRESS addr, int *value) {
	// read memory location
	if (!pagetable->read_word(addr, value)) {
		// call kernel with error status
		context.setpc(startpc);
		kernel.event(1, addr);
		return FALSE;
	}
	return TRUE;
}

BOOLEAN CPU::write_byte(ADDRESS addr, char value) {
	// read memory location
	if (!pagetable->write_byte(addr, value)) {
		// call kernel with error status
		context.setpc(startpc);
		kernel.event(1, addr);
		return FALSE;
	}
	return TRUE;
}

BOOLEAN CPU::write_word(ADDRESS addr, int value) {
	// write memory location
	if (!pagetable->write_word(addr, value)) {
		// call kernel with error status
		context.setpc(startpc);
		kernel.event(1, addr);
		return FALSE;
	}
	return TRUE;
}

BOOLEAN CPU::get_pc_byte(char *value) {
	ADDRESS addr;

	// get address PC is pointing to
	addr = context.getpc();
	// read memory location
	if (!read_byte(addr, value))
		return FALSE;
	// update PC
	context.setpc(context.getpc() + 1);  // increment PC with 1
	return TRUE;
}

BOOLEAN CPU::get_pc_word(int *value) {
	ADDRESS addr;

	// get address PC is pointing to
	addr = context.getpc();
	// read memory location
	if (!read_word(addr, value))
		return FALSE;
	// update PC
	context.setpc(context.getpc() + 2);  // increment PC with 2
	return TRUE;
}

BOOLEAN CPU::get_ea(int *value) {
	int imm, lval, rval;
	char lreg, rreg;

	// get immediate value
	if (!get_pc_word(&imm))
		return FALSE;
	// get left register
	if (!get_pc_byte(&lreg))
		return FALSE;
	// get right register
	if (!get_pc_byte(&rreg))
		return FALSE;
	// if lreg non-zero, then read contence
	lval = lreg ? context.getreg(lreg) : 0;
	// if rreg non-zero, then read contence
	rval = rreg ? context.getreg(rreg) : 0;
	// merge everything into a result
	*value = imm + lval + rval;
	return TRUE;
}



void disp_opc(unsigned char *arr) {
	enum {
		_ILLEGAL = 0x00, _MOVW, _STOW, _LEA, _MOVB, _STOB,
		_SVC     = 0x0A,
		_CMP     = 0x10, _MOVR, _SUB, _ADD, _MUL, _DIV, _MOD, _BOR, _BAND, _XOR, _LSL, _LSR, _NOT, _NEG,
		_JSB     = 0x20, _RSB, _x22, _PSHR, _POPR, _PSHB, _PSHW, _PSHA,
		_LE      = 0x63, _LT, _GE, _GT, _NE, _EQ,
		_JMP     = 0x6F
	};

	static const char *opc_name[0x70];

	if (!opc_name[_ADD]) {
		opc_name[_ADD]  = "ADD";
		opc_name[_SUB]  = "SUB";
		opc_name[_MUL]  = "MUL";
		opc_name[_DIV]  = "DIV";
		opc_name[_MOD]  = "MOD";
		opc_name[_BOR]  = "BOR";
		opc_name[_XOR]  = "XOR";
		opc_name[_BAND] = "AND";
		opc_name[_LSR]  = "LSR";
		opc_name[_LSL]  = "LSL";
		opc_name[_MOVR] = "MOVR";

		opc_name[_ILLEGAL] = "ILLEGAL";
		opc_name[_NEG]     = "NEG";
		opc_name[_NOT]     = "NOT";
		opc_name[_EQ]      = "BEQ";
		opc_name[_NE]      = "BNE";
		opc_name[_LT]      = "BLT";
		opc_name[_LE]      = "BLE";
		opc_name[_GT]      = "BGT";
		opc_name[_GE]      = "BGE";
		opc_name[_MOVB]    = "LODB";
		opc_name[_MOVW]    = "LODW";
		opc_name[_LEA]     = "LODA";
		opc_name[_CMP]     = "CMP";
		opc_name[_STOB]    = "STOB";
		opc_name[_STOW]    = "STOW";
		opc_name[_JMP]     = "JMP";
		opc_name[_JSB]     = "JSB";
		opc_name[_RSB]     = "RSB";
		opc_name[_PSHR]    = "PSHR";
		opc_name[_POPR]    = "POPR";
		opc_name[_PSHB]    = "PSHB";
		opc_name[_PSHW]    = "PSHW";
		opc_name[_PSHA]    = "PSHA";
		opc_name[_SVC]     = "SVC";
	};

	switch (arr[0]) {
		case _ADD:
		case _SUB:
		case _MUL:
		case _DIV:
		case _MOD:
		case _BOR:
		case _XOR:
		case _BAND:
		case _LSR:
		case _LSL:
		case _MOVR:
		case _CMP:
			printf("%s R%d,R%d\n", opc_name[arr[0]], arr[1], arr[2]);
			break;
		case _NEG:
		case _NOT:
			printf("%s R%d\n", opc_name[arr[0]], arr[1]);
			break;
		case _ILLEGAL:
		case _RSB:
			printf("%s\n", opc_name[arr[0]]);
			break;
		case _EQ:
		case _NE:
		case _LT:
		case _LE:
		case _GT:
		case _GE:
		case _JMP:
		case _JSB:
		case _PSHB:
		case _PSHW:
		case _PSHA:
			printf("%s %02x%02x(R%d,R%d)\n", opc_name[arr[0]], arr[1], arr[2], arr[3], arr[4]);
			break;
		case _MOVB:
		case _MOVW:
		case _LEA:
		case _STOB:
		case _STOW:
			printf("%s R%d,%02x%02x(R%d,R%d)\n", opc_name[arr[0]], arr[1], arr[2], arr[3], arr[4], arr[5]);
			break;
		case _PSHR:
		case _POPR:
		case _SVC:
			printf("%s %02x%02x\n", opc_name[arr[0]], arr[1], arr[2]);
			break;
		default:
			printf("OPC_%d\n", arr[0]);
			break;
	}
}

void CPU::tick() {
	int addr, spaddr, val, result;
	int i, R0, R1;
	char opcode, lreg, rreg, ch;

	if (0) {
		int val, base, i, j;
		unsigned char arr[6];

		// generate a register dump
		printf("PC:%04x", context.getpc() & 0xffff);
		for (i = 0; i < 8; i++)
			printf(" R%02d:%04x", i, context.getreg(i) & 0xffff);
		printf("\n");
		printf("CC:%04x", alu.getcc());
		for (i = 8; i < 16; i++)
			printf(" R%02d:%04x", i, context.getreg(i) & 0xffff);
		printf("\n");

		for (j = 0; j < 6; j++)
			pagetable->read_byte(context.getpc() + j, (char*)&arr[j]);
		disp_opc(arr);

	}


	// get next opcode
	startpc = context.getpc();
	if (!get_pc_byte(&opcode))
		return;

	// decode opcode
	switch (opcode) {
		case LDW: // LDW R,M
			// load operands
			if (get_pc_byte(&lreg) && get_ea(&addr))
				// read word pointed to by EA addr
				if (read_word(addr, &val))
					// save found value into register (set CC in alu)
					context.setreg(lreg, alu.calc(LDR, 0, val));
			break;
		case STW: // STW R,M
			// load operands
			if (get_pc_byte(&lreg) && get_ea(&addr))
				// write word pointed to by EA addr (set CC in alu)
				write_word(addr, alu.calc(LDR, 0, context.getreg(lreg)));
			break;
		case LDA: // LDA R,M
			// load operands
			if (get_pc_byte(&lreg) && get_ea(&addr))
				// save found value into register (set CC in alu)
				context.setreg(lreg, alu.calc(LDR, 0, addr));
			break;
		case LDB: // LDB R,M
			// load operands
			if (get_pc_byte(&lreg) && get_ea(&addr))
				// read word pointed to by EA addr
				if (read_byte(addr, &ch))
					// save found value into register (set CC in alu)
					context.setreg(lreg, alu.calc(LDR, 0, ch));
			break;
		case STB: // STB R,M
			// load operands
			if (get_pc_byte(&lreg) && get_ea(&addr))
				// write word pointed to by EA addr (set CC in alu)
				write_byte(addr, alu.calc(LDR, 0, context.getreg(lreg)));
			break;
		case SVC: // SVC I
			// load SVC opcode
			if (get_pc_word(&val)) {
				// get contence of R0 and R1
				R0 = context.getreg(0);
				R1 = context.getreg(1);
				if (kernel.supervisor(val, &R0, &R1)) {
					// restore values of R0 and R1
					context.setreg(0, R0);
					context.setreg(1, R1);
				} else {
					// SVC not implemented
					context.setpc(startpc);
					kernel.event(2, startpc);
				}
			}
			break;
		case CMP: // CMP R,R
			// load operands
			if (get_pc_byte(&lreg) && get_pc_byte(&rreg))
				// execute a dummy SUB to set CC bits
				result = alu.calc(SUB, context.getreg(lreg), context.getreg(rreg));
			break;
		case LDR: // LDR R,R
		case SUB: // SUB R,R
		case ADD: // ADD R,R
		case MUL: // MUL R,R
		case DIV: // DIV R,R
		case MOD: // MOD R,R
		case OR : // OR  R,R
		case AND: // AND R,R
		case XOR: // XOR R,R
		case ASL: // ASL R,R
		case ASR: // ASR R,R
			// load operands
			if (get_pc_byte(&lreg) && get_pc_byte(&rreg)) {
				// execute operation, and set CC bits
				result = alu.calc(opcode, context.getreg(lreg), context.getreg(rreg));
				// store result in register
				context.setreg(lreg, result); // store result
			}
			break;
		case NOT: // NOT R
		case NEG: // NEG R
			// load operands
			if (get_pc_byte(&lreg)) {
				// execute operation, and set CC bits
				result = alu.calc(opcode, context.getreg(lreg), 0);
				// store result in register
				context.setreg(lreg, result); // store result
			}
			break;
		case JSB: // JSB M
			// load operands
			if (get_ea(&addr)) {
				// execute EA calculation of -(sp)
				spaddr = context.getreg(15) - 2;
				context.setreg(15, spaddr);
				// write PC to (SP)
				if (write_word(spaddr, context.getpc()))
					// setup new PC
					context.setpc(addr);
			}
			break;
		case RSB: // RSB
			// execute EA calculation of (sp)+
			spaddr = context.getreg(15);
			context.setreg(15, spaddr + 2);
			// read (SP) into val
			if (read_word(spaddr, &val))
				// jump to saved address
				context.setpc(val);
			break;
		case PSHR: // PSHR I
			// load register mask
			if (get_pc_word(&addr)) {
				for (i = 0; i < 15; i++)
					// test if bit in mask
					if (addr & (1 << i)) {
						// push register onto stack

						// execute EA calculation of -(sp)
						spaddr = context.getreg(15) - 2;
						context.setreg(15, spaddr);
						// write reg to (SP)
						if (!write_word(spaddr, context.getreg(i)))
							break;  // error occurred, stop loop
					}
			}
			break;
		case POPR: // POPR I
			// load register mask
			if (get_pc_word(&addr)) {
				for (i = 14; i >= 0; i--)
					// test if bit in mask
					if (addr & (1 << i)) {
						// pop register from stack

						// execute EA calculation of (sp)+
						spaddr = context.getreg(15);
						context.setreg(15, spaddr + 2);
						// load reg from (SP)
						if (!read_word(spaddr, &val))
							break;  // error occurred, stop loop
						else
							context.setreg(i, val);
					}
			}
			break;
		case PSHB: // PSHB M
			// load operands
			if (get_ea(&addr)) {
				// get contence of addr
				if (read_byte(addr, &ch)) {
					// execute EA calculation of -(sp)
					spaddr = context.getreg(15) - 2;
					context.setreg(15, spaddr);
					// write word pointed to by EA to (SP)
					write_word(spaddr, alu.calc(LDR, 0, ch));
				}
			}
			break;
		case PSHW: // PSHW M
			// load operands
			if (get_ea(&addr)) {
				// get contence of addr
				if (read_word(addr, &val)) {
					// execute EA calculation of -(sp)
					spaddr = context.getreg(15) - 2;
					context.setreg(15, spaddr);
					// write word pointed to by EA to (SP)
					write_word(spaddr, alu.calc(LDR, 0, val));
				}
			}
			break;
		case PSHA: // PSHA M
			// load operands
			if (get_ea(&addr)) {
				// execute EA calculation of -(sp)
				spaddr = context.getreg(15) - 2;
				context.setreg(15, spaddr);
				// write EA to (SP)
				write_word(spaddr, alu.calc(LDR, 0, addr));
			}
			break;
		case BLE: // BLE M
		case BLT: // BLT M
		case BGE: // BGE M
		case BGT: // BGT M
		case BNE: // BNE M
		case BEQ: // BEQ M
			if (get_ea(&addr))           // get jump address
				if (alu.test_cc(opcode))
					context.setpc(addr);     // jump if condition valid
			break;
		case JMP: // JMP M
			if (get_ea(&addr))           // get jump address
				context.setpc(addr);       // always jump
			break;
		default:  // unimplemented opcode
			context.setpc(startpc);
			kernel.event(2, startpc);
			break;
	}
}

BOOLEAN CPU::loadfile(char *fname) {
	pagetable->loadfile(fname);
}

void CPU::load_context(CPU_CONTEXT *context, PAGE_TABLE *pagetable) {
	// load context
	CPU::context = *context;
	CPU::pagetable = pagetable;
	// store CC in ALU
	alu.setcc(CPU::context.getcc());
}

void CPU::save_context(CPU_CONTEXT *context) {
	// retrieve CC from alu
	CPU::context.setcc(alu.getcc());
	// return context
	*context = CPU::context;
}

void CPU::pushArgs(char *argv[]) {

	int BPW = 2;
	int i, len, strbase, argvbase, argc, sp;
	char *arg;

	sp = context.getreg(15);

	// count needed string space
	len = argc = 0;
	for (i = 0; argv[i]; i++)
		len += strlen(argv[argc++]) + 2;

	// align
	len = (len + 1) & ~1;

	// string base
	sp = (sp - len) & 0xffff;
	strbase = sp;
	// argv base
	sp = (sp - (argc + 1) * BPW) & 0xffff;
	argvbase = sp;

	// copy strings
	for (i = 0; i < argc; i++) {
		write_word(argvbase, strbase);
		argvbase += BPW;
		for (arg = argv[i]; *arg; arg++)
			write_byte(strbase++, *arg);
		write_byte(strbase++, 0);
	}

	// terminator NULL
	write_word(argvbase, 0);

	// push argc
	sp -= BPW;
	write_word(sp, argc);

	// push argv
	sp -= BPW;
	write_word(sp, sp + 4);

	// push size of push-frame
	sp -= BPW;
	write_word(sp, 3 * BPW);

	// push return address
	sp -= BPW;
	write_word(sp, 0);

	context.setreg(15, sp);
}
