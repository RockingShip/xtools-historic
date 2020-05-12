#ifndef CONTEXT_HPP
#define CONTEXT_HPP

class CPU_CONTEXT {

private :
	int regs[16];
	int pc;
	int cc;

public :
	CPU_CONTEXT();

	int getreg(int reg);

	int getpc();

	int getcc();

	void setreg(int reg, int value);

	void setpc(int value);

	void setcc(int value);

};

#endif
