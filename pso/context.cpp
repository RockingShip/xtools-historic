#include "context.hpp"
#include "system.hpp"

CPU_CONTEXT::CPU_CONTEXT() {
	int i;

	for (i = 0; i < 16; i++)
		regs[i] = 0;
	pc = 0;
	cc = 0;
}

int CPU_CONTEXT::getreg(int reg) {
	return regs[reg & 0xF];
}

int CPU_CONTEXT::getpc() {
	return pc;
}

int CPU_CONTEXT::getcc() {
	return cc;
}

void CPU_CONTEXT::setreg(int reg, int value) {
	regs[reg & 0xF] = value;
}

void CPU_CONTEXT::setpc(int value) {
	pc = value;
}

void CPU_CONTEXT::setcc(int value) {
	cc = value;
}
