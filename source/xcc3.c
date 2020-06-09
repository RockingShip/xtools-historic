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

/*
 * @date 2020-05-23 17:09:01
 *
 * Test if lval is a constant stored in `lval[LVALUE]`
 */
isConstant(register int lval[])
{
	return (lval[LTYPE] == ADDRESS && lval[LNAME] == 0 && lval[LREG] == 0);
}

/*
 * @date 2020-05-24 14:02:47
 *
 * Test if lval is a register stored in `lval[LREG]`
 */
isRegister(register int lval[])
{
	return (lval[LTYPE] == ADDRESS && lval[LNAME] == 0 && lval[LVALUE] == 0);
}

/*
 * Test if storage is BPW large. "int*" "char*" "int".
 */
isWORD(register int lval[]) {
	return lval[LPTR]  || lval[LSIZE] == BPW;
}

/*
 * Test if "int*".
 */
isINTPTR(register int lval[]) {
	return lval[LPTR]  && lval[LSIZE] == BPW;
}

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

	for (i = 2; i < REGMAX; ++i) {
		mask = 1 << i;
		if (!((regresvd | reguse) & mask)) {
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

	if (lval[LTYPE] == ADDRESS) {
		// try to relocate offset to register
		if (lval[LREG] == 0) {
			if (lval[LVALUE] == 1) {
				lval[LREG] = REG_1;
				lval[LVALUE] = 0;
			} else if (lval[LVALUE] == BPW) {
				lval[LREG] = REG_BPW;
				lval[LVALUE] = 0;
			}
		}

		// assign resulting register
		if (reg > 0) {
			// loaded into fixed register
			freelval(lval);
		} else if (lval[LNAME] || lval[LVALUE]) {
			// need code to evaluate, result always in new register
			freelval(lval);
			reg = allocreg();
		} else if (reg == 0 && (reglock & (1 << lval[LREG]))) {
			// reserved/locked register to writable register
			reg = allocreg();
		} else if (lval[LNAME] == 0 && lval[LVALUE] == 0) {
			// already in register
			return;
		} else {
			// reuse register
			reg = lval[LREG];
		}

		// generate code
		if (lval[LNAME] || lval[LVALUE]) {
			gencode_lval(TOK_LDA, reg, lval);
		} else if (lval[LREG] == 0) {
			gencode_R(TOK_LDR, reg, REG_0);
		} else {
			gencode_R(TOK_LDR, reg, lval[LREG]);
		}

		// Modify lval
		lval[LTYPE] = ADDRESS;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = reg;
	} else if (lval[LTYPE] == MEMORY) {
		freelval(lval);
		if (reg <= 0)
			reg = allocreg();
		gencode_lval(isWORD(lval) ? TOK_LDW : TOK_LDB, reg, lval);

		lval[LTYPE] = ADDRESS;
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

		lval[LTYPE] = ADDRESS;
		lval[LPTR] = 0;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = reg;
	} else
		fatal("unimplemented");
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
xplng1(int (*hier)(), register int start, register int lval[]) {
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

		// Load rval
		if (!(*hier)(rval)) {
			exprerr();
			return 1;
		}

		// Put rval into a register
		// Generate code
		if (isConstant(lval) && isConstant(rval)) {
			lval[LVALUE] = calc(lval[LVALUE], hier_oper[entry], rval[LVALUE]);
		} else {
			loadlval(lval, 0);
			loadlval(rval, -1);

			// Execute operation and release rval
			gencode_R(hier_oper[entry], lval[LREG], rval[LREG]);
			freelval(rval);
		}
	}
}

/*
 * generic processing for <lval> <comparison> <rval>
 */
xplng2(int (*hier)(), register int start, register int lval[]) {
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
		lval[LTRUE] = lval[LFALSE] = 0;
	}
	return 1;
}

/*
 * Auto increment/decrement
 */
step(register int pre, register int lval[], register int post) {
	int dest[LLAST];
	register int reg;

	if (!isRegister(lval) && lval[LTYPE] != MEMORY)
		expected("lvalue");

	// Copy lval
	dest[LTYPE] = lval[LTYPE];
	dest[LPTR] = lval[LPTR];
	dest[LSIZE] = lval[LSIZE];
	dest[LNAME] = lval[LNAME];
	dest[LVALUE] = lval[LVALUE];
	dest[LREG] = lval[LREG];

	if (isRegister(lval)) {
		gencode_R((pre | post), lval[LREG], isINTPTR(lval) ? REG_BPW : REG_1);
		if (post) {
			reg = allocreg();
			gencode_R(TOK_LDR, reg, lval[LREG]);
			gencode_R((TOK_ADD + TOK_SUB - post), reg, isINTPTR(lval) ? REG_BPW : REG_1);
			freelval(lval);
			lval[LREG] = reg;
		}
	} else {
		reg = allocreg();
		loadlval(lval, reg);
		gencode_R((pre | post), lval[LREG], isINTPTR(lval) ? REG_BPW : REG_1);
		gencode_lval(isWORD(dest) ? TOK_STW : TOK_STB, lval[LREG], dest);
		if (post) {
			gencode_R((TOK_ADD + TOK_SUB - post), reg, isINTPTR(lval) ? REG_BPW : REG_1);
			lval[LREG] = reg;
		}
	}
}

/*
 * Inject assembler into output and catch return value
 */
doasm(register int lval[]) {
	needtoken("(");

	if (!match("\"")) {
		expected("string");
	} else {
		fputc('\t', outhdl);
		while (ch && (ch != '"'))
			fputc(litchar(), outhdl);
		fputc('\n', outhdl);

		gch(); // skip terminator
	}

	blanks();
	needtoken(")");

	// make R1 available
	lval[LTYPE] = ADDRESS;
	lval[LPTR] = 0;
	lval[LSIZE] = BPW;
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
		expression(lval);
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
	for (i = symidx - 1; i >= 0; --i) {
		sym = &syms[i * ILAST];
		if (sym[ISYM] == sname) {
			lval[LTYPE] = sym[ITYPE];
			lval[LPTR] = sym[IPTR];
			lval[LSIZE] = sym[ISIZE];
			lval[LNAME] = sym[INAME];
			lval[LVALUE] = sym[IVALUE];
			lval[LREG] = sym[IREG];

			return 1;
		}
	}

	// test for reserved words
	if (sname == argcid) {
		// generate (2(AP)-BPW)/BPW
		lval[LTYPE] = ADDRESS;
		lval[LPTR] = 0;
		lval[LSIZE] = BPW;
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
	lval[LNAME] = sname;
	lval[LVALUE] = 0;
	lval[LREG] = 0;

	return 1;
}

/*
 * Do a hierarchical evaluation
 */
expr_postfix(register int lval[]) {
	int lval2[LLAST], sav_csp;
	register int argc, reg;

	if (!primary(lval))
		return 0;
	if (match("[")) { // [subscript]
		if (!lval[LPTR])
			error("can't subscript");
		else if (!expression(lval2))
			error("need subscript");
		else {
			if (isConstant(lval2)) {
				if (lval[LTYPE] == MEMORY)
					loadlval(lval, 0); // load if pointer
				// Subscript is a constant
				lval[LVALUE] += lval2[LVALUE] * lval[LSIZE];
			} else {
				// Subscript is a variable/complex-expression
				if (lval[LTYPE] == MEMORY)
					loadlval(lval, 0); // load if pointer
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
			lval[LTYPE] = MEMORY;
			--lval[LPTR]; // deference pointer
		}
		needtoken("]");
	}
	if (match("(")) { // function (...)
		if (lval[LTYPE] != FUNCTION)
			error("Illegal function");

		argc = BPW;
		sav_csp = csp;
		blanks();
		while (ch != ')') {
			// Get expression
			expr_assign(lval2);
			if (isConstant(lval2)) {
				gencode_I(TOK_PSHA, -1, lval2[LVALUE]);
			} else {
				if (lval2[LTYPE] == BRANCH)
					loadlval(lval2, 0);
				freelval(lval2);
				// Push onto stack
				if (lval2[LTYPE] != MEMORY)
					gencode_lval(TOK_PSHA, -1, lval2);
				else
					gencode_lval(isWORD(lval2) ? TOK_PSHW : TOK_PSHB, -1, lval2);
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
		gencode_lval(TOK_JSB, -1, lval);
		freelval(lval);

		// Pop args
		csp = 0; // `gencode_M` will subtract csp from sp.
		gencode_M(TOK_LDA, REG_SP, 0, argc, REG_SP);

		csp = sav_csp;

		lval[LTYPE] = ADDRESS;
		lval[LPTR] = 0;
		lval[LSIZE] = BPW;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = REG_RETURN;
	}

	if (match("++")) {
		step(0, lval, TOK_ADD);
	} else if (match("--")) {
		step(0, lval, TOK_SUB);
	}

	return 1;
}

expr_unary(register int lval[]) {

	if (match("++")) {
		if (!expr_unary(lval)) {
			exprerr();
			return 0;
		}
		step(TOK_ADD, lval, 0);
	} else if (match("--")) {
		if (!expr_unary(lval)) {
			exprerr();
			return 0;
		}
		step(TOK_SUB, lval, 0);
	} else if (match("~")) {
		if (!expr_unary(lval)) {
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
		if (!expr_unary(lval)) {
			exprerr();
			return 0;
		}
		if (isConstant(lval))
			lval[LVALUE] = !lval[LVALUE];
		else if (lval[LTYPE] == BRANCH && !lval[LTRUE]) {
			/*
			 * @date 2020-06-07 19:16:22
			 * FIXED: Need to invert last instruction AND all prior by swapping T/F labels.
			 */
			// invert opcode in peephole
			lval[LVALUE] = negop(lval[LVALUE]);
			// swap labels
			int sav;
			sav = lval[LTRUE];
			lval[LTRUE] = lval[LFALSE];
			lval[LFALSE] = sav;
		} else {
			// convert CC bits into a BRANCH
			loadlval(lval, 0);
			freelval(lval);
			lval[LTYPE] = BRANCH;
			lval[LVALUE] = TOK_BNE;
			lval[LTRUE] = lval[LFALSE] = 0;
		}
	} else if (match("-")) {
		if (!expr_unary(lval)) {
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
		if (!expr_unary(lval)) {
			exprerr();
			return 0;
		}
	} else if (match("*")) {
		if (!expr_unary(lval)) {
			exprerr();
			return 0;
		}
		if (!lval[LPTR])
			error("can't dereference");
		else if (lval[LTYPE] == FUNCTION) {
			// dereference pointer to function
			int reg;
			freelval(lval);
			reg = allocreg();
			gencode_lval(TOK_LDW, reg, lval);
			--lval[LPTR];
			lval[LNAME] = 0;
			lval[LVALUE] = 0;
			lval[LREG] = reg;
		} else {
			if (lval[LTYPE] == MEMORY)
				loadlval(lval, 0); // load if pointer
			lval[LTYPE] = MEMORY;
			--lval[LPTR]; // deference pointer
		}
	} else if (match("&")) {
		if (!expr_unary(lval)) {
			exprerr();
			return 0;
		}
		if (lval[LTYPE] != MEMORY)
			expected("lvalue");
		else {
			lval[LTYPE] = ADDRESS;
			++lval[LPTR];
		}
	} else {
		if (!expr_postfix(lval))
			return 0;
	}
	return 1;
}

expr_muldiv(int lval[]) {
	return xplng1(expr_unary, 24, lval);
}

expr_addsub(int lval[]) {
	return xplng1(expr_muldiv, 21, lval);
}

expr_shift(int lval[]) {
	return xplng1(expr_addsub, 18, lval);
}

expr_relational(int lval[]) {
	return xplng2(expr_shift, 13, lval);
}

expr_equality(int lval[]) {
	return xplng2(expr_relational, 10, lval);
}

expr_and(int lval[]) {
	return xplng1(expr_equality, 8, lval);
}

expr_xor(int lval[]) {
	return xplng1(expr_and, 6, lval);
}

expr_or(int lval[]) {
	return xplng1(expr_xor, 4, lval);
}

expr_land(int lval[]) {
	register int lbl;
	int once;

	// Load lval
	if (!expr_or(lval))
		return 0;

	once = 1;
	while (1) {
		if (!omatch("&&"))
			return 1;

		if (once) {
			// One time only: process lval and jump

			// lval must be BRANCH
			if (lval[LTYPE] != BRANCH) {
				loadlval(lval, 0);
				freelval(lval);
				lval[LTYPE] = BRANCH;
				lval[LVALUE] = TOK_BEQ;
				lval[LTRUE] = lval[LFALSE] = 0;
			}

			if (!lval[LFALSE])
				lval[LFALSE] = ++nxtlabel;
			lbl = lval[LFALSE];

			// Mark done
			once = 0;
		}

		// postprocess last lval
		gencode_L(lval[LVALUE], lval[LFALSE]);
		if (lval[LTRUE])
			fprintf(outhdl, "_%d:", lval[LTRUE]);

		// Load next lval
		if (!expr_or(lval)) {
			exprerr();
			return 1;
		}

		// lval must be BRANCH
		if (lval[LTYPE] != BRANCH) {
			loadlval(lval, 0);
			freelval(lval);
			lval[LTYPE] = BRANCH;
			lval[LVALUE] = TOK_BEQ;
			lval[LTRUE] = lval[LFALSE] = 0;
		}

		if (lval[LFALSE])
			fprintf(outhdl, "_%d=_%d\n", lval[LFALSE], lbl);
		lval[LFALSE] = lbl;
	}
}

expr_lor(int lval[]) {
	register int lbl;
	int once;

	// Load lval
	if (!expr_land(lval))
		return 0;

	once = 1;
	while (1) {
		if (!omatch("||"))
			return 1;

		if (once) {
			// One time only: process lval and jump

			// lval must be BRANCH
			if (lval[LTYPE] != BRANCH) {
				loadlval(lval, 0);
				freelval(lval);
				lval[LTYPE] = BRANCH;
				lval[LVALUE] = TOK_BEQ;
				lval[LTRUE] = lval[LFALSE] = 0;
			}

			if (!lval[LTRUE])
				lval[LTRUE] = ++nxtlabel;
			lbl = lval[LTRUE];

			// Mark done
			once = 0;
		}

		// postprocess last lval
		gencode_L(negop(lval[LVALUE]), lval[LTRUE]);
		if (lval[LFALSE])
			fprintf(outhdl, "_%d:", lval[LFALSE]);

		// Load next lval
		if (!expr_land(lval)) {
			exprerr();
			return 1;
		}

		// lval must be BRANCH
		if (lval[LTYPE] != BRANCH) {
			loadlval(lval, 0);
			freelval(lval);
			lval[LTYPE] = BRANCH;
			lval[LVALUE] = TOK_BEQ;
			lval[LTRUE] = lval[LFALSE] = 0;
		}

		if (lval[LTRUE])
			fprintf(outhdl, "_%d=_%d\n", lval[LTRUE], lbl);
		lval[LTRUE] = lbl;
	}
}

expr_ternary(register int lval[]) {
	register int lbl, reg;

	if (!expr_lor(lval))
		return 0;
	if (!match("?"))
		return 1;

	// If not a BRANCH then convert it into one
	if (lval[LTYPE] != BRANCH) {
		loadlval(lval, 0);
		freelval(lval);
		lval[LTYPE] = BRANCH;
		lval[LVALUE] = TOK_BEQ;
		lval[LTRUE] = lval[LFALSE] = 0;
	}
	// alloc labels (copy to variable because of `loadlval()`
	int lfalse;
	lfalse = lval[LFALSE];
	if (!lfalse)
		lfalse = ++nxtlabel;

	// process 'true' variant
	gencode_L(lval[LVALUE], lfalse);
	if (lval[LTRUE])
		fprintf(outhdl, "_%d:", lval[LTRUE]);
	expression(lval);
	loadlval(lval, reg = allocreg()); // Needed to assign a dest reg

	needtoken(":");
	// jump to end
	lbl = ++nxtlabel;
	gencode_L(TOK_JMP, lbl);

	// process 'false' variant
	fprintf(outhdl, "_%d:", lfalse);
	if (!expr_ternary(lval))
		exprerr();
	else
		loadlval(lval, reg); // Needed for result to occupy same reg

	fprintf(outhdl, "_%d:", lbl);

	// resulting type is undefined, so modify LTYPE
	lval[LTYPE] = ADDRESS;
	lval[LPTR] = 0;

	return 1;
}

expr_assign(register int lval[]) {
	int rval[LLAST], dest[LLAST];
	register int oper;

	if (!expr_ternary(lval))
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
	if (!isRegister(lval) && lval[LTYPE] != MEMORY)
		expected("lvalue");

	// Get rval
	if (!expr_assign(rval)) {
		exprerr();
		return 1;
	}
	loadlval(rval, -1);

	if (oper == -1) {
		if (isRegister(lval))
			gencode_R(TOK_LDR, lval[LREG], rval[LREG]);
		else
			gencode_lval(isWORD(lval) ? TOK_STW : TOK_STB, rval[LREG], lval);
		// continue with value stored in register `rval[LREG]`
		freelval(lval);
		lval[LTYPE] = ADDRESS;
		lval[LNAME] = 0;
		lval[LVALUE] = 0;
		lval[LREG] = rval[LREG];
	} else {
		// Copy lval
		dest[LTYPE] = lval[LTYPE];
		dest[LPTR] = lval[LPTR];
		dest[LSIZE] = lval[LSIZE];
		dest[LNAME] = lval[LNAME];
		dest[LVALUE] = lval[LVALUE];
		dest[LREG] = lval[LREG];

		// load lval into reg, modify it with rval and copy result into dest
		loadlval(lval, allocreg()); // don't reuse regs for dest
		gencode_R(oper, lval[LREG], rval[LREG]);
		freelval(rval);
		// write-back
		if (isRegister(dest))
			gencode_R(TOK_LDR, dest[LREG], lval[LREG]);
		else
			gencode_lval(isWORD(lval) ? TOK_STW : TOK_STB, lval[LREG], dest);
	}

	return 1;
}

/*
 * Load a numerical expression separated by comma's
 */
expression(register int lval[]) {

	if (!expr_assign(lval)) {
		expected("expression");
		return 0;
	}

	while (match(",")) {
		freelval(lval);
		if (!expr_assign(lval))
			expected("expression");
	}
	return 1;
}

/*
 * Load a constant expression
 */
constexpr(register int *val) {
	int lval[LLAST];

	if (!expr_ternary(lval))
		return 0;
	if (isConstant(lval)) {
		*val = lval[LVALUE];
		return 1;
	}
	expected("constant expression");
	freelval(lval);
	return 0;
}

/*
 * Load a constant value
 */
constant(register int lval[]) {
	lval[LTYPE] = ADDRESS;
	lval[LPTR] = 0;
	lval[LSIZE] = 0;
	lval[LNAME] = 0;
	lval[LVALUE] = 0;
	lval[LREG] = 0;

	if (number(&lval[LVALUE]))
		return 1;

	// character
	if (match("'")) {
		register int i;

		i = 0;
		while (ch && (ch != '\''))
			i = (i << 8) + litchar();
		gch();

		lval[LVALUE] = i;
		return 1;
	}

	// start of string
	if (!match("\""))
		return 0;

	int lbl, prevseg;
	lbl = ++nxtlabel;
	prevseg = currseg;

	toseg(TEXTSEG);
	fprintf(outhdl, "_%d:\t.DCB\t\"", lbl);

	while (ch && (ch != '"')) {
		if (ch == '\\') {
			fputc(ch, outhdl);
			gch();
		}
		fputc(ch, outhdl);
		gch();
	}
	fprintf(outhdl, "\",0\n");
	gch(); // skip terminator

	toseg(prevseg);

	// Convert to array of char
	lval[LTYPE] = ADDRESS;
	lval[LPTR] = 1;
	lval[LSIZE] = 1;
	lval[LNAME] = -lbl;
	lval[LVALUE] = 0;
	lval[LREG] = 0;

	return 1;
}

/*
 * Get a numerical constant
 */
number(register int *val) {
	register int i, minus;

	i = minus = 0;
	if (!(ctype[ch] & CISDIGIT))
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

	/*
	 * @date 2020-06-09 02:02:25
	 * 0x8000 becomes 32768 (32-bits) and -32768 (16-bits).
	 */
	// sign extend
	i |= -(i & (1 << SBIT));

	*val = i;
	return 1;
}

/*
 * Return current literal character and bump lptr
 */
litchar() {
	register int i, oct;

	if (ch != '\\' || nch == 0)
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
