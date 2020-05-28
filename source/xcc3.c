/*
 * X-C-Compiler/Assembler/Linker/Archiver
 */

/*
 * MIT License
 *
 * Copyright (c) 1991 xyzzy@rockingship.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//*
//* X-C-Compiler.  Part 3, Expression evaluation
//*

#define EXTERN extern
#include "xcc.h"

/* test if lval is stored in a word */

#define lval_ISBPW  (lval[LPTR]  || (lval[LSIZE] == BPW))
#define lval2_ISBPW (lval2[LPTR] || (lval2[LSIZE] == BPW))
#define dest_ISBPW  (dest[LPTR]  || (dest[LSIZE] == BPW))
#define lval_ISIPTR (lval[LPTR]  && (lval[LSIZE] == BPW))

/*
 * Get the inverse of an compare
 */
negop(register int op) {
	switch (op) {
		case TOK_BEQ: return TOK_BNE;
		case TOK_BNE: return TOK_BEQ;
		case TOK_BGT: return TOK_BLE;
		case TOK_BLT: return TOK_BGE;
		case TOK_BGE: return TOK_BLT;
		case TOK_BLE: return TOK_BGT;
		default: return op; // No negation
	}
}

/*
 * Process a constant evaluation
 */
calc(register int left, int oper, int right) {
	switch (oper) {
		case TOK_OR : return (left | right);
		case TOK_XOR: return (left ^ right);
		case TOK_AND: return (left & right);
		case TOK_BEQ : return (left == right);
		case TOK_BNE : return (left != right);
		case TOK_BLE : return (left <= right);
		case TOK_BGE : return (left >= right);
		case TOK_BLT : return (left < right);
		case TOK_BGT : return (left > right);
		case TOK_LSR: return (left >> right);
		case TOK_LSL: return (left << right);
		case TOK_ADD: return (left + right);
		case TOK_SUB: return (left - right);
		case TOK_MUL: return (left * right);
		case TOK_DIV: return (left / right);
		case TOK_MOD: return (left % right);
		default: return 0;
	}
}

/*
 * Allocate a free register
 */
allocreg() {
	register int i, mask;

	for (i = 2; i < REGMAX; i++) {
		mask = 1 << i;
		if (~(regresvd | reguse) & mask) {
			regsum |= reguse |= mask;
			return i;
		}
	}
	error("out of registers");
	return 2; // First modifiable reg
}

/*
 * Return a register into the free list
 */
freereg(register int reg) {
	register int mask;

	mask = 1 << reg;
	if (!(regresvd & mask)) {
		if (reguse & mask) {
			reguse &= ~mask;
			reglock &= ~mask;
		} else
			error("double register deallocation");
	}
}

/*
 * Load a lval into a register
 *
 * If 'reg' == -1 then register is mode is 'register read'. This
 * mode is same as 'reg' == 0 except reserved registers are not
 * copied to 'free' registers
 */
loadlval(register int lval[], register int reg) {
	register int srcreg;

	// Sign extend to fix being called with negative constant when copiled with "-Dint=long"
	reg |= -(reg & (1 << SBIT));

	if (isConstant(lval)) {
		// test for a predefined register
		if (reg > 0)
			gencode_I(TOK_LDA, reg, lval[LVALUE]);
		else {
			if (lval[LVALUE] == 0)
				srcreg = REG_0;
			else if (lval[LVALUE] == 1)
				srcreg = REG_1;
			else if (lval[LVALUE] == BPW)
				srcreg = REG_BPW;
			else {
				srcreg = allocreg();
				gencode_I(TOK_LDA, srcreg, lval[LVALUE]);
			}

			if (reg == -1)
				reg = srcreg;
			else if (srcreg < REG_0)
				reg = srcreg;
			else {
				reg = allocreg();
				gencode_R(TOK_LDR, reg, srcreg);
			}
		}

		// Modify lval
		lval[LTYPE] = EXPR;
		lval[LPTR] = 0;
		lval[LEA] = EA_ADDR;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = reg;
	} else if (lval[LTYPE] == BRANCH) {
		int lblX;
		lblX = ++nxtlabel;

		if (reg <= 0)
			reg = allocreg();
		if (!lval[LFALSE])
			lval[LFALSE] = ++nxtlabel;
		gencode_L(lval[LVALUE], lval[LFALSE]);
		if (lval[LTRUE])
			fprintf(outhdl, "_%d:", lval[LTRUE]);
		gencode_R(TOK_LDR, reg, REG_1);
		gencode_L(TOK_JMP, lblX);
		fprintf(outhdl, "_%d:", lval[LFALSE]);
		gencode_R(TOK_LDR, reg, REG_0);
		fprintf(outhdl, "_%d:", lblX);

		lval[LTYPE] = EXPR;
		lval[LPTR] = 0;
		lval[LEA] = EA_ADDR;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = reg;
	} else if (lval[LEA] == EA_IND) {
		freelval(lval);
		if (reg <= 0)
			reg = allocreg();
		gencode_M(lval_ISBPW ? TOK_LDW : TOK_LDB, reg, lval[LNAME], lval[LVALUE], lval[LREG]);

		lval[LEA] = EA_ADDR;
		// NOTE: lval[LPTR] can be non-zero
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = reg;
	} else if (!isRegister(lval)) {
		freelval(lval);
		if (reg <= 0)
			reg = allocreg();
		gencode_M(TOK_LDA, reg, lval[LNAME], lval[LVALUE], lval[LREG]);

		lval[LEA] = EA_ADDR;
		lval[LPTR] = 0;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = reg;
	} else if (((reg > 0) && (lval[LREG] != reg)) ||
		   (regresvd & (1 << lval[LREG])) ||
		   ((reg != -1) && (reglock & (1 << lval[LREG])))) {
		freelval(lval);
		if (reg <= 0)
			reg = allocreg();
		gencode_R(TOK_LDR, reg, lval[LREG]);

		lval[LREG] = reg;
	}
}

/*
 * Free all registers assigned to a lval
 */
freelval(register int lval[]) {
	if (isConstant(lval) || lval[LTYPE] == BRANCH)
		return;
	if (!(reglock & (1 << lval[LREG])))
		freereg(lval[LREG]);
}

/*
 * generic processing for <lval> { <operation> <rval> }
 */
xplng1(register int (*hier)(), register int start, register int lval[]) {
	register char *cptr, entry;
	int rval[LLAST];

	// Load lval
	if (!(*hier)(lval))
		return 0;

	while (1) {
		// Locate operation
		entry = start;
		while (1) {
			if (!(cptr = hier_str[entry]))
				return 1;
			if (omatch(cptr))
				break;
			++entry;
		}

		// Put lval into a register
		if (!lval[LPTR] && (lval[LTYPE] == FUNCTION))
			error("Invalid function use");

		// Load rval
		if (!(*hier)(rval)) {
			exprerr();
			return 1;
		}

		// Put rval into a register
		if (!rval[LPTR] && (rval[LTYPE] == FUNCTION))
			error("Invalid function use");

		// Generate code
		if (isConstant(lval) && isConstant(rval)) {
			lval[LVALUE] = calc(lval[LVALUE], hier_oper[entry], rval[LVALUE]);
		} else {
			loadlval(lval, 0);
			loadlval(rval, -1);

			// Execute operation and release rval
			gencode_R(hier_oper[entry], lval[LREG], rval[LREG]);
			freelval(rval);

			// Modify lval
			lval[LTYPE] = EXPR;
			lval[LPTR] = 0;
			lval[LEA] = EA_ADDR;
		}
	}
}

/*
 * generic processing for <lval> <comparison> <rval>
 */
xplng2(register int (*hier)(), register int start, register int lval[]) {
	register char *cptr, entry;
	int rval[LLAST];

	// Load lval
	if (!(*hier)(lval))
		return 0;

	// Locate operation
	entry = start;
	while (1) {
		if (!(cptr = hier_str[entry]))
			return 1;
		if (omatch(cptr))
			break;
		++entry;
	}

	// Load rval
	if (!(*hier)(rval)) {
		exprerr();
		return 1;
	}

	// Generate code
	if (isConstant(lval) && isConstant(rval)) {
		lval[LVALUE] = calc(lval[LVALUE], hier_oper[entry], rval[LVALUE]);
	} else {
		loadlval(lval, -1);
		loadlval(rval, -1);

		// Compare and release values
		gencode_R(TOK_CMP, lval[LREG], rval[LREG]);
		freelval(lval);
		freelval(rval);

		// Change lval to "BRANCH"
		lval[LTYPE] = BRANCH;
		lval[LVALUE] = negop(hier_oper[entry]);
		lval[LFALSE] = lval[LTRUE] = 0;
	}
	return 1;
}

/*
 * generic processing for <lval> { ('||' | '&&') <rval> }
 */
xplng3(register int (*hier)(), register int start, register int lval[]) {
	register char *cptr, entry;
	register int lbl;
	int once;

	// Load lval
	if (!(*hier)(lval))
		return 0;

	once = 1;
	entry = start;
	while (1) {
		// Locate operation
		while (1) {
			if (!(cptr = hier_str[entry]))
				return 1;
			if (omatch(cptr))
				break;
			++entry;
		}

		// Put lval into a register
		if (!lval[LPTR] && (lval[LTYPE] == FUNCTION))
			error("Invalid function use");

		if (once) {
			// One time only: process lval and jump

			// lval must be BRANCH
			if (lval[LTYPE] != BRANCH) {
				loadlval(lval, 0);
				freelval(lval);
				lval[LTYPE] = BRANCH;
				lval[LVALUE] = TOK_BEQ;
				lval[LFALSE] = lval[LTRUE] = 0;
			}

			if (hier_oper[entry] == TOK_AND) {
				if (!lval[LFALSE])
					lval[LFALSE] = ++nxtlabel;
				lbl = lval[LFALSE];
			} else {
				if (!lval[LTRUE])
					lval[LTRUE] = ++nxtlabel;
				lbl = lval[LTRUE];
			}

			// Mark done
			once = 0;
		}

		// postprocess last lval
		if (hier_oper[entry] == TOK_AND) {
			gencode_L(lval[LVALUE], lval[LFALSE]);
			if (lval[LTRUE])
				fprintf(outhdl, "_%d:", lval[LTRUE]);
		} else {
			gencode_L(negop(lval[LVALUE]), lval[LTRUE]);
			if (lval[LFALSE])
				fprintf(outhdl, "_%d:", lval[LFALSE]);
		}

		// Load next lval
		if (!(*hier)(lval)) {
			exprerr();
			return 1;
		}

		// Put lval into a register
		if (!lval[LPTR] && (lval[LTYPE] == FUNCTION))
			error("Invalid function use");

		// lval must be BRANCH
		if (lval[LTYPE] != BRANCH) {
			loadlval(lval, 0);
			freelval(lval);
			lval[LTYPE] = BRANCH;
			lval[LVALUE] = TOK_BEQ;
			lval[LFALSE] = lval[LTRUE] = 0;
		}

		if (hier_oper[entry] == TOK_AND) {
			if (lval[LFALSE])
				fprintf(outhdl, "_%d=_%d\n", lval[LFALSE], lbl);
			lval[LFALSE] = lbl;
		} else {
			if (lval[LTRUE])
				fprintf(outhdl, "_%d=_%d\n", lval[LTRUE], lbl);
			lval[LTRUE] = lbl;
		}
	}
}

/*
 * Auto increment/decrement
 */
step(register int pre, register int lval[], register int post) {
	int dest[LLAST];
	register int reg;

	if (lval[LTYPE] == EXPR || isConstant(lval) || lval[LTYPE] == BRANCH)
		error("non-modifiable variable");

	// Copy lval
	dest[LTYPE] = lval[LTYPE];
	dest[LPTR] = lval[LPTR];
	dest[LSIZE] = lval[LSIZE];
	dest[LEA] = lval[LEA];
	dest[LNAME] = lval[LNAME];
	dest[LVALUE] = lval[LVALUE];
	dest[LREG] = lval[LREG];

	if (isRegister(lval)) {
		gencode_R((pre | post), lval[LREG], lval_ISIPTR ? REG_BPW : REG_1);
		if (post) {
			reg = allocreg();
			gencode_R(TOK_LDR, reg, lval[LREG]);
			gencode_R((TOK_ADD + TOK_SUB - post), reg, lval_ISIPTR ? REG_BPW : REG_1);
			freelval(lval);
			lval[LREG] = reg;
		}
	} else {
		reg = allocreg();
		loadlval(lval, reg);
		gencode_R((pre | post), lval[LREG], lval_ISIPTR ? REG_BPW : REG_1);
		gencode_M(dest_ISBPW ? TOK_STW : TOK_STB, lval[LREG], dest[LNAME], dest[LVALUE], dest[LREG]);
		if (post) {
			gencode_R((TOK_ADD + TOK_SUB - post), reg, lval_ISIPTR ? REG_BPW : REG_1);
			lval[LREG] = reg;
		}
	}
}

/*
 * Inject assembler into output and catch return value
 */
doasm(register int lval[]) {
	needtoken("(");

	if (!qstr()) {
		error("string expected");
	} else {
		register int i;

		fputc('\t', outhdl);
		for (i = 0; i < litinx - 1; i++) // one less for trailing zero
			fputc(litq[i], outhdl);
		fputc('\n', outhdl);
	}

	blanks();
	needtoken(")");

	// make R1 available
	lval[LTYPE] = EXPR;
	lval[LPTR] = 0;
	lval[LSIZE] = BPW;
	lval[LEA] = EA_ADDR;
	lval[LNAME] = 0;
	lval[LVALUE] = 0;
	lval[LREG] = REG_RETURN;
}

/*
 * Load primary expression
 */
primary(register int lval[]) {
	register int *sym, i;
	int sname, len;

	if (match("(")) {  // (expression,...)
		expression(lval, 1);
		needtoken(")");
		return 1;
	}

	// test for "asm()"
	if (amatch("asm")) {
		doasm(lval);
		return 1;
	}

	// load identifier
	if (!(len = dohash(lptr, &sname)))
		return constant(lval);
	bump(len); // Skip identifier

	// identifier. Scan in reverse order of creation.
	for (i = symidx - 1; i >= 0; i--) {
		sym = &syms[i * ILAST];
		if (sym[INAME] == sname) {
			lval[LTYPE] = sym[ITYPE];
			lval[LPTR] = sym[IPTR];
			lval[LSIZE] = sym[ISIZE];
			lval[LNAME] = 0;
			lval[LVALUE] = 0;
			lval[LREG] = 0;

			if (sym[ICLASS] == CONSTANT) {
				lval[LTYPE] = EXPR;
				lval[LPTR] = 0;
				lval[LSIZE] = 0;
				lval[LEA] = EA_ADDR;
				lval[LVALUE] = sym[IVALUE];
			} else if (sym[ICLASS] == REGISTER) {
				lval[LEA] = EA_ADDR;
				lval[LREG] = sym[IVALUE];
			} else if (sym[ICLASS] == AP_AUTO) {
				lval[LEA] = EA_IND;
				lval[LVALUE] = sym[IVALUE];
				lval[LREG] = REG_AP;
			} else if (sym[ICLASS] == SP_AUTO) {
				lval[LEA] = EA_IND;
				lval[LVALUE] = sym[IVALUE];
				lval[LREG] = REG_SP;
			} else {
				lval[LEA] = EA_IND;
				lval[LNAME] = sname;
			}

			// functions/arrays are addresses
			if ((sym[ITYPE] == FUNCTION || sym[ITYPE] == ARRAY) && !sym[IPTR]) {
				if (lval[LEA] != EA_IND)
					fatal("ARRAY not EA_IND\n");
				lval[LEA] = EA_ADDR;
			}

			return 1;
		}
	}

	// test for reserved words
	if (sname == argcid) {
		// generate (2(AP)-BPW)/BPW
		lval[LTYPE] = EXPR;
		lval[LPTR] = 0;
		lval[LSIZE] = BPW;
		lval[LEA] = EA_ADDR;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = allocreg();

		gencode_M(TOK_LDW, lval[LREG], 0, BPW, REG_AP);
		gencode_R(TOK_SUB, lval[LREG], REG_BPW);
		gencode_R(TOK_DIV, lval[LREG], REG_BPW);
		return 1;
	} else if (sname == argvid) {
		exprerr();
		return 0;
	}

	// todo: too soon, drop pre-processor first
	// warning("undefined identifier");

	// could be external function names
	lval[LTYPE] = FUNCTION;
	lval[LPTR] = 0;
	lval[LSIZE] = BPW;
	lval[LEA] = EA_ADDR;
	lval[LNAME] = sname;
	lval[LVALUE] = 0;
	lval[LREG] = 0;

	return 1;
}

/*
 * Do a hierarchical evaluation
 */
hier14(register int lval[]) {
	int lval2[LLAST], sav_csp;
	register int argc, reg;

	if (!primary(lval))
		return 0;
	if (match("[")) { // [subscript]
		if (!(lval[LTYPE] == ARRAY || (lval[LTYPE] == VARIABLE && lval[LPTR])))
			error("can't subscript");
		else if (!hier1(lval2))
			error("need subscript");
		else {
			if (isConstant(lval2)) {
				if (lval[LEA] == EA_IND)
					loadlval(lval, 0); // make LVALUE available
				// Subscript is a constant
				lval[LVALUE] += lval2[LVALUE] * lval[LSIZE];
			} else {
				// Subscript is a variable/complex-expression
				if (lval[LEA] == EA_IND)
					loadlval(lval, 0); // make LREG2 available
				loadlval(lval2, 0);
				if (lval[LSIZE] == BPW)
					gencode_R(TOK_MUL, lval2[LREG], REG_BPW); // size index
				if (!lval[LREG])
					lval[LREG] = lval2[LREG];
				else {
					/*
					 * @date 2020-05-23 01:14:18
					 * This is an important location.
					 * having a second indirect register removes the following code.
					 * however, a single register is a much simpler implementation.
					 */
					if ((1<<lval[LREG]) & reglock) {
						// register in lval is locked and needs to be made writable
						freelval(lval);
						reg = allocreg();
						gencode_R(TOK_LDR, reg, lval[LREG]);
						lval[LREG] = reg;
					}
					gencode_R(TOK_ADD, lval[LREG], lval2[LREG]);
					freelval(lval2);
				}
			}
			// Update data type
			lval[LPTR] = 0;
			lval[LEA] = EA_IND;
		}
		needtoken("]");
	}
	if (match("(")) { // function (...)
		if (lval[LPTR] || (lval[LTYPE] != FUNCTION))
			error("Illegal function");

		argc = BPW;
		sav_csp = csp;
		blanks();
		while (ch != ')') {
			// Get expression
			expression(lval2, 0);
			if (isConstant(lval2)) {
				gencode_I(TOK_PSHA, -1, lval2[LVALUE]);
			} else {
				if (lval2[LTYPE] == BRANCH)
					loadlval(lval2, 0);
				freelval(lval2);
				// Push onto stack
				if (lval2[LEA] != EA_IND)
					gencode_M(TOK_PSHA, -1, lval2[LNAME], lval2[LVALUE], lval2[LREG]);
				else
					gencode_M(lval2_ISBPW ? TOK_PSHW : TOK_PSHB, -1, lval2[LNAME], lval2[LVALUE], lval2[LREG]);
			}
			// increment ARGC
			csp -= BPW;
			argc += BPW;

			if (!match(","))
				break;
		}
		needtoken(")");
		// Push ARGC
		gencode_I(TOK_PSHA, -1, argc);

		// call
		gencode_M(TOK_JSB, -1, lval[LNAME], lval[LVALUE], lval[LREG]);
		freelval(lval);

		// Pop args
		csp = 0; // `gencode_M` will subtract csp from sp.
		gencode_M(TOK_LDA, REG_SP, 0, argc, REG_SP);

		csp = sav_csp;

		lval[LTYPE] = EXPR;
		lval[LPTR] = 0;
		lval[LSIZE] = BPW;
		lval[LEA] = EA_ADDR;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = REG_RETURN;
	}
	return 1;
}

hier13(register int lval[]) {

	if (match("++")) {
		if (!hier13(lval)) {
			exprerr();
			return 0;
		}
		step(TOK_ADD, lval, 0);
	} else if (match("--")) {
		if (!hier13(lval)) {
			exprerr();
			return 0;
		}
		step(TOK_SUB, lval, 0);
	} else if (match("~")) {
		if (!hier13(lval)) {
			exprerr();
			return 0;
		}
		if (isConstant(lval))
			lval[LVALUE] = ~lval[LVALUE];
		else {
			loadlval(lval, 0);
			gencode_R(TOK_NOT, -1, lval[LREG]);
		}
	} else if (match("!")) {
		if (!hier13(lval)) {
			exprerr();
			return 0;
		}
		if (isConstant(lval))
			lval[LVALUE] = !lval[LVALUE];
		else if (lval[LTYPE] == BRANCH && !lval[LTRUE]) {
			// @date 	2020-05-19 20:21:57
			// bugfix: can only negate opcode if no other opcodes were generated
			lval[LVALUE] = negop(lval[LVALUE]);
		} else {
			// convert CC bits into a BRANCH
			loadlval(lval, 0);
			freelval(lval);
			lval[LTYPE] = BRANCH;
			lval[LVALUE] = TOK_BNE;
			lval[LFALSE] = lval[LTRUE] = 0;
		}
	} else if (match("-")) {
		if (!hier13(lval)) {
			exprerr();
			return 0;
		}
		if (isConstant(lval))
			lval[LVALUE] = -lval[LVALUE];
		else {
			loadlval(lval, 0);
			gencode_R(TOK_NEG, -1, lval[LREG]);
		}
	} else if (match("+")) {
		if (!hier13(lval)) {
			exprerr();
			return 0;
		}
	} else if (match("*")) {
		if (!hier13(lval)) {
			exprerr();
			return 0;
		}
		if (!lval[LPTR] || isConstant(lval) || lval[LTYPE] == BRANCH)
			error("Illegal address");
		else {
			if (lval[LEA] == EA_IND)
				loadlval(lval, 0);
			lval[LPTR] = 0;
			lval[LEA] = EA_IND;
		}
	} else if (match("&")) {
		if (!hier13(lval)) {
			exprerr();
			return 0;
		}
		if (lval[LEA] != EA_IND || isConstant(lval)  || lval[LTYPE] == BRANCH)
			error("Illegal address");
		else {
			lval[LTYPE] = EXPR;
			lval[LSIZE] = lval_ISBPW ? BPW : 1;
			lval[LPTR] = 1;
			lval[LEA] = EA_ADDR;
		}
	} else {
		if (!hier14(lval))
			return 0;
		if (match("++")) {
			step(0, lval, TOK_ADD);
		} else if (match("--")) {
			step(0, lval, TOK_SUB);
		}
	}
	return 1;
}

hier12(int lval[]) {
	return xplng1(hier13, 24, lval);
}

hier11(int lval[]) {
	return xplng1(hier12, 21, lval);
}

hier10(int lval[]) {
	return xplng1(hier11, 18, lval);
}

hier9(int lval[]) {
	return xplng2(hier10, 13, lval);
}

hier8(int lval[]) {
	return xplng2(hier9, 10, lval);
}

hier7(int lval[]) {
	return xplng1(hier8, 8, lval);
}

hier6(int lval[]) {
	return xplng1(hier7, 6, lval);
}

hier5(int lval[]) {
	return xplng1(hier6, 4, lval);
}

hier4(int lval[]) {
	return xplng3(hier5, 2, lval);
}

hier3(int lval[]) {
	return xplng3(hier4, 0, lval);
}

hier2(register int lval[]) {
	register int lbl, reg;

	if (!hier3(lval))
		return 0;
	if (!match("?"))
		return 1;

	// If not a BRANCH then convert it into one
	if (lval[LTYPE] != BRANCH) {
		loadlval(lval, 0);
		freelval(lval);
		lval[LTYPE] = BRANCH;
		lval[LVALUE] = TOK_BEQ;
		lval[LFALSE] = lval[LTRUE] = 0;
	}
	// alloc labels
	if (!lval[LFALSE])
		lval[LFALSE] = ++nxtlabel;

	// process 'true' variant
	gencode_L(lval[LVALUE], lval[LFALSE]);
	if (lval[LTRUE])
		fprintf(outhdl, "_%d:", lval[LTRUE]);
	expression(lval, 1);
	loadlval(lval, reg = allocreg()); // Needed to assign a dest reg

	needtoken(":");
	// jump to end
	lbl = ++nxtlabel;
	gencode_L(TOK_JMP, lbl);

	// process 'false' variant
	fprintf(outhdl, "_%d:", lval[LFALSE]);
	if (!hier1(lval))
		exprerr();
	else
		loadlval(lval, reg); // Needed for result to occupy same reg

	fprintf(outhdl, "_%d:", lbl);

	// resulting type is undefined, so modify LTYPE
	lval[LTYPE] = EXPR;
	lval[LPTR] = 0;
	lval[LEA] = EA_ADDR;

	return 1;
}

hier1(register int lval[]) {
	int rval[LLAST], dest[LLAST];
	register int oper;

	if (!hier2(lval))
		return 0;

	// Test for assignment
	if (omatch("=")) oper = -1;
	else if (match("|=")) oper = TOK_OR;
	else if (match("^=")) oper = TOK_XOR;
	else if (match("&=")) oper = TOK_AND;
	else if (match("+=")) oper = TOK_ADD;
	else if (match("-=")) oper = TOK_SUB;
	else if (match("*=")) oper = TOK_MUL;
	else if (match("/=")) oper = TOK_DIV;
	else if (match("%=")) oper = TOK_MOD;
	else if (match(">>=")) oper = TOK_LSR;
	else if (match("<<=")) oper = TOK_LSL;
	else
		return 1;

	// test if lval modifiable
	if (lval[LTYPE] == EXPR || isConstant(lval) || lval[LTYPE] == BRANCH)
		error("Inproper lvalue");

	// Get rval
	if (!hier1(rval)) {
		exprerr();
		return 1;
	}
	loadlval(rval, -1);

	if (oper == -1) {
		if (isRegister(lval))
			gencode_R(TOK_LDR, lval[LREG], rval[LREG]);
		else {
			gencode_M(lval_ISBPW ? TOK_STW : TOK_STB, rval[LREG], lval[LNAME], lval[LVALUE], lval[LREG]);
			freelval(lval);
		}
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = rval[LREG];
	} else {
		// Copy lval
		dest[LTYPE] = lval[LTYPE];
		dest[LPTR] = lval[LPTR];
		dest[LSIZE] = lval[LSIZE];
		dest[LEA] = lval[LEA];
		dest[LNAME] = lval[LNAME];
		dest[LVALUE] = lval[LVALUE];
		dest[LREG] = lval[LREG];

		// load lval into reg, modify it with rval and copy result into dest
		loadlval(lval, allocreg()); // don't reuse regs for dest
		gencode_R(oper, lval[LREG], rval[LREG]);
		freelval(rval);
		if (isRegister(dest))
			gencode_R(TOK_LDR, dest[LREG], lval[LREG]);
		else
			gencode_M(lval_ISBPW ? TOK_STW : TOK_STB, lval[LREG], dest[LNAME], dest[LVALUE], dest[LREG]);
	}

	// resulting type is undefined, so modify LTYPE
	lval[LTYPE] = EXPR;
	lval[LPTR] = 0;
	lval[LEA] = EA_ADDR;

	return 1;
}

/*
 * Load a numerical expression seperated by comma's
 */
expression(register int lval[], int comma) {

	if (!hier1(lval)) {
		error("expression required");
		junk();
	}

	while (comma && match(",")) {
		freelval(lval);
		if (!hier1(lval)) {
			error("expression required");
			junk();
		}
	}
}

/*
 * @date 2020-05-23 17:09:01
 *
 * Test if lval is a constant stored in `lval[LVALUE]`
 */
isConstant(register int lval[])
{
	return (lval[LTYPE] == EXPR && lval[LPTR] == 0 && lval[LEA] == EA_ADDR && lval[LNAME] == 0 && lval[LREG] == 0);
}

/*
 * @date 2020-05-24 14:02:47
 *
 * Test if lval is a register stored in `lval[LREG]`
 */
isRegister(register int lval[])
{
	if (lval[LTYPE] == VARIABLE || lval[LTYPE] == ARRAY || lval[LTYPE] == EXPR) {
		if (lval[LEA] == EA_ADDR && lval[LNAME] == 0 && lval[LVALUE] == 0)
			return 1;
	}
	return 0;
}

/*
 * Load a constant expression
 */
constexpr(register int val[]) {
	int lval[LLAST];

	if (!hier1(lval))
		return 0;
	if (isConstant(lval)) {
		*val = lval[LVALUE];
		return 1;
	}
	error("must be a constant expression");
	freelval(lval);
	return 0;
}


/*
 * Load a constant value
 */
constant(register int lval[]) {
	lval[LTYPE] = EXPR;
	lval[LPTR] = 0;
	lval[LSIZE] = 0;
	lval[LEA] = EA_ADDR;
	lval[LNAME] = 0;
	lval[LVALUE] = 0;
	lval[LREG] = 0;

	if (number(&lval[LVALUE]))
		return 1;
	if (pstr(&lval[LVALUE]))
		return 1;
	if (qstr()) {
		// Convert to char pointer
		lval[LTYPE] = ARRAY;
		lval[LPTR] = 1;
		lval[LSIZE] = 1;
		lval[LEA] = EA_ADDR;
		lval[LNAME] = - ++nxtlabel;
		lval[LVALUE] = 0;
		lval[LREG] = 0;
		// Generate data
		toseg(DATASEG);
		fprintf(outhdl, "_%d:", - lval[LNAME]);
		dumplits(1);
		toseg(prevseg);
		return 1;
	}
	return 0;
}

/*
 * Get a numerical constant
 */
number(register int *val) {
	register int i, minus;

	i = minus = 0;
	if (~ctype[ch] & CISDIGIT)
		return 0;
	if ((ch == '0') && (toupper(nch) == 'X')) {
		bump(2);
		while (1) {
			if (ctype[ch] & CISDIGIT)
				i = i * 16 + (gch() - '0');
			else if (ctype[ch] & CISLOWER)
				i = i * 16 + (gch() - 'a' + 10);
			else if (ctype[ch] & CISUPPER)
				i = i * 16 + (gch() - 'A' + 10);
			else
				break;
		}
	} else {
		while (ctype[ch] & CISDIGIT)
			i = i * 10 + (gch() - '0');
	}
	*val = i;
	return 1;
}

/*
 * Get a character constant
 */
pstr(int *val) {
	register int i;

	i = 0;
	if (!match("'"))
		return 0;
	while (ch && (ch != '\''))
		i = (i << 8) + litchar();
	gch();
	*val = i;
	return 1;
}

/* 
 * Get a string constant
 */
qstr() {
	if (!match("\""))
		return 0;

	litinx = 0;
	while (ch && (ch != '"'))
		addlits(litchar(), 1);
	gch(); // skip terminator
	addlits(0, 1);
	return 1;
}

/*
 * Return current literal character and bump lptr
 */
litchar() {
	register int i, oct;

	if ((ch != '\\') || (nch == 0))
		return gch();
	gch();
	switch (ch) {
	case 'b': // bell
		gch();
		return 8;
	case 'f': // form-feed
		gch();
		return 12;
	case 'n': // newline
		gch();
		return 10;
	case 'r': // carriage return
		gch();
		return 13;
	case 't': // horizontal tab
		gch();
		return 9;
	case 'v': // vertical tab
		gch();
		return 11;
	}
	i = 0;
	oct = 0;
	while ((ch >= '0') && (ch <= '7')) {
		oct = (oct << 3) + gch() - '0';
		++i;
	}
	return i ? oct : gch();
}

/*
 * Add a value to the literal pool
 */
addlits(register int val, register int size) {
	if (size == BPW) {
		litq[litinx++] = val >> 8;
		if (litinx >= LITMAX)
			fatal("Literal queue overflow");
	}
	litq[litinx++] = val;
	if (litinx >= LITMAX)
		fatal("Literal queue overflow");
}

/*
 * dump the literal pool
 */
dumplits(int size) {
	register int i, j;

	if (!litinx)
		return;

	i = 0;
	while (i < litinx) {
		if (size == 1)
			fprintf(outhdl, "\t.DCB ");
		else
			fprintf(outhdl, "\t.DCW ");

		for (j = 0; j < 16; j++) {
			if (size == 1)
				fprintf(outhdl, "%d", (i >= litinx) ? 0 : litq[i]);
			else
				fprintf(outhdl, "%d", (i >= litinx) ? 0 : ((litq[i + 0] << 8) | (litq[i + 1] & 0xff)));
			i += size;
			if (i >= litinx)
				break;
			if (j != 15)
				fprintf(outhdl, ",");
		}
		fprintf(outhdl, "\n");
	}
	litinx = 0;
}
