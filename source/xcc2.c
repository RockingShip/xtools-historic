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
//* X-C-Compiler.  Part 2, Syntax Control
//*

#define EXTERN extern
#include "xcc.h"

/*
 * Declare/define a procedure argument
 */
declarg(int scope, register int class, register int argnr) {
	int size, sname, len, ptr, type, cnt, reg;
	register int *sym, i;

	// get size of type
	if (amatch("char"))
		size = 1; // char is stored as int on stack
	else if (amatch("int"))
		size = 2;
	else
		return 0;

	// scan definitions
	while (1) {
		blanks();

		if (match("*"))
			ptr = 1;
		else if (match("(")) {
			ptr = 2;
			needtoken("*");
		} else
			ptr = 0;

		if (!(len = dohash(lptr, &sname)))
			illname();
		if (len)
			bump(len);

		for (i = scope; i < symidx; i++) {
			sym = &syms[i * ILAST];
			if (sym[INAME] == sname) {
				multidef();
				break;
			}
		}

		type = VARIABLE;

		if (ptr == 2) {
			match(")");
			if (match("(")) {
				if (match(")"))
					type = FUNCTION;
				else
					error("bad (*)()");
			}
		}

		cnt = 1; // Number of elements
		if (match("[")) {
			if (ptr)
				error("array of pointers not supported");
			if (type != VARIABLE)
				error("array type not supported");

			type = ARRAY;
			ptr = 1; // address of array (passed as argument) is pushed on stack

			// get number of elements
			if (constexpr(&cnt))
				error("arraysize not allowed");

			// force single dimension
			needtoken("]");
		}

		// add symbol to symboltable
		if (symidx >= SYMMAX)
			fatal("identifier table overflow");
		sym = &syms[symidx++ * ILAST];
		sym[INAME] = sname;
		sym[ITYPE] = type;
		sym[IPTR] = ptr;
		sym[ICLASS] = class;
		sym[ISIZE] = size;
		sym[IVALUE] = (-argnr + 1) * BPW;

		// modify location if chars. Chars are pushed as words, adjust to point to lo-byte
		if (size == 1 && !ptr)
			sym[IVALUE] += BPW - 1;

		// only one
		break;
	}
}

/*
 * General global definitions
 */
declvar(int scope, register int class) {
	int size, sname, len, ptr, type, cnt;
	register int *sym, i;

	// get size of type
	if (amatch("char"))
		size = 1;
	else if (amatch("int"))
		size = 2;
	else
		return 0;

	// scan definitions
	while (1) {
		blanks();

		ptr = (match("(*") || match("*"));
		if (!(len = dohash(lptr, &sname)))
			illname();
		if (len)
			bump(len);

		for (i = scope; i < symidx; i++) {
			if (syms[i * ILAST + INAME] == sname)
				break;
		}
		if (i < symidx)
			multidef();

		match(")");
		type = VARIABLE;
		if (match("()"))
			type = FUNCTION;

		cnt = 1; // Number of elements
		if (match("[")) {
			if (ptr)
				error("array of pointers not supported");
			if (type != VARIABLE)
				error("array type not supported");
			if (class == REGISTER)
				error("register array not supported");

			type = ARRAY;
			ptr = 0;

			// get number of elements
			if (!constexpr(&cnt))
				cnt = 0;
			else if (cnt < 0)
				warning("warning: negative size");

			// force single dimension
			needtoken("]");
		}

		// add symbol to symboltable
		if (symidx >= SYMMAX)
			fatal("identifier table overflow");
		sym = &syms[symidx++ * ILAST];
		sym[INAME] = sname;
		sym[ITYPE] = type;
		sym[IPTR] = ptr;
		sym[ICLASS] = class;
		sym[ISIZE] = size;
		sym[IVALUE] = 0;

		// Now generate code
		if (match("=")) {
			int lval[LLAST];

			toseg(DATASEG);
			fprintf(outhdl, "_");
			symname(sname);
			fprintf(outhdl, ":");
			if (class != STATIC)
				fprintf(outhdl, ":");

			// assign value to variable
			litinx = 0;
			if (!match("{")) {
				// single value
				if (!hier1(lval))
					error("expected a constant");
				if (lval[LTYPE] == CONSTANT) {
					if (ptr || (type == ARRAY))
						error("cannot assign constant to pointer or array");
					addlits(lval[LVALUE], size);
				} else if (lval[LTYPE] == LABEL) {
					if ((size != 1) || (!ptr && (type != ARRAY)))
						error("must assign to char pointers or arrays");
					// at this point, literal queue has been filled with string
				} else {
					error("expected a constant");
					litinx = 2;
				}
			} else {
				// multiple values
				if (!ptr && (type != ARRAY))
					error("must assign to pointers or arrays");

				// get values
				while (1) {
					--cnt;
					if (!hier1(lval))
						error("expected a constant");
					if (lval[LTYPE] == CONSTANT) {
						addlits(lval[LVALUE], size);
					} else if (lval[LTYPE] == LABEL) {
						error("multiple strings not allowed");
						litinx = 2;
					} else {
						error("expected a constant");
					}

					// test for reloop
					if (match(","))
						continue;
					else if (match("}"))
						break;
					else
						error("expected }");
				}
			}

			// dump literal pool
			if (ptr) {
				fprintf(outhdl, "\t.DCW\t_%d", ++nxtlabel);
				fprintf(outhdl, "_%d:", nxtlabel);
			}
			dumplits(size);

			// if array and not all elements have been supplied, then pad
			if (!ptr && (cnt > 0))
				if (size == 1)
					fprintf(outhdl, "\t.DSB\t%d\n", cnt);
				else
					fprintf(outhdl, "\t.DSW\t%d\n", cnt);

		} else if (sym[ICLASS] == REGISTER) {
			sym[IVALUE] = allocreg();
			reglock |= (1 << sym[IVALUE]);

		} else if (sym[ICLASS] == SP_AUTO) {
			if (ptr)
				sym[IVALUE] = (csp -= BPW);
			else if (size == 1)
				sym[IVALUE] = (csp -= cnt);
			else
				sym[IVALUE] = (csp -= cnt * BPW);

		} else if (sym[ICLASS] != EXTERNAL) {
			toseg(UDEFSEG);
			fprintf(outhdl, "_");
			symname(sname);
			fprintf(outhdl, ":");
			if (class != STATIC)
				fprintf(outhdl, ":");
			if (ptr)
				fprintf(outhdl, "\t.DSW\t1\n");
			else if (size == 1)
				fprintf(outhdl, "\t.DSB\t%d\n", cnt);
			else
				fprintf(outhdl, "\t.DSW\t%d\n", cnt);
		}

		// test for more
		if (!match(","))
			break;
	}

	// done
	ns();
	return 1;
}

//
// General global definitions
//
declenum() {
	int *sym;
	int seqnr;
	register int i;
	int len, sname;
	int lval[LLAST];

	needtoken("{");

	// get values
	seqnr = 0;
	while (1) {
		blanks();

		if (ch == '}')
			break;

		if (!(len = dohash(lptr, &sname))) {
			illname();
			kill();
			return;
		}
		bump(len);

		// add symbol to symboltable
		for (i = symidx - 1; i >= 0; i--) {
			sym = &syms[i * ILAST];
			if (sym[INAME] == sname)
				break;
		}
		if (i >= 0)
			multidef();
		else if (symidx >= SYMMAX)
			fatal("identifier table overflow");
		sym = &syms[symidx++ * ILAST];

		sym[INAME] = sname;
		sym[ITYPE] = CONSTANT; // todo: this should actually be VARIABLE
		sym[IPTR] = 0;
		sym[ICLASS] = 0; // todo: this should actually be CONSTANT
		sym[IVALUE] = 0;
		sym[ISIZE] = 0;

		if (match("=")) {
			expression(lval, 0);
			if (lval[LTYPE] != CONSTANT) {
				error("expected expression");
				return;
			}
			seqnr = lval[LVALUE];
		}

		sym[IVALUE] = seqnr++;

		if (!match(","))
			break;
	}
	needtoken("}");
	ns();
}

/*
 * open an include file
 */
doinclude() {
	register char *p;

	white();
	if (*lptr != '"')
		error("filename expected");
	else if (inchdl)
		error("Nested #include not allowed");
	else {
		// Modify sourceline to extract filename
		p = incfn;
		gch(); // Skip delimiter
		while ((*p++ = gch()) != '"');
		*--p = 0; // Remove delimiter

		// Open file
		inchdl = mustopen(incfn, "r");
	}

	// make next read come from new file (if open)
	kill();
}

/*
 * Declare/define a macro
 */
declmac() {
	int sname;
	register int len;
	register int *mptr;

	white();
	if (!(len = dohash(lptr, &sname)))
		error("identifier expected");
	else if (macinx >= MACMAX)
		fatal("#define overflow");
	else {
		// copy macroname
		mptr = &mac[macinx++ * MLAST];
		mptr[MNAME] = sname;
		mptr[MEXPAND] = &macq[macqinx];
		// Copy expansion
		bump(len);
		white();
		while (ch) {
			if (macqinx < MACQMAX)
				macq[macqinx++] = ch;
			gch();
		}
		if (macqinx < MACQMAX)
			macq[macqinx++] = 0; // Terminator
		if (macqinx >= MACQMAX)
			fatal("#define string overflow");
	}
}

/*
 *
 */
declfunc() {
	int returnlbl, len, sname, lbl1, lbl2, inireg, sav_argc, scope;
	register int *sym, i, numarg;

	returnlbl = ++nxtlabel;
	reguse = regsum = reglock = 1 << REG_AP; // reset all registers
	csp = -1; // reset stack
	swinx = 1;
	toseg(CODESEG);
	if (verbose)
		printf("%s\n", lptr);

	// get procedure name
	if (!(len = dohash(lptr, &sname))) {
		error("illegal function name or declaration");
		kill();
		return;
	}
	bump(len);

	scope = symidx;

	for (i = 0; i < symidx; i++) {
		sym = &syms[i * ILAST];
		if (sym[INAME] == sname)
			break;
	}
	if (i < symidx && sym[ICLASS] != AUTOEXT)
		multidef();
	if (symidx >= SYMMAX)
		fatal("identifier table overflow");

	if (i >= symidx)
		sym = &syms[symidx++ * ILAST];

	// (re)define procedure
	sym[INAME] = sname;
	sym[ITYPE] = FUNCTION;
	sym[IPTR] = 0;
	sym[ICLASS] = GLOBAL;
	sym[IVALUE] = 0;
	sym[ISIZE] = BPW;
	// Generate global label
	fprintf(outhdl, "_");
	symname(sname);
	fprintf(outhdl, "::");
	gencode_R(TOK_LDR, 1, REG_SP);
	lbl1 = ++nxtlabel;
	fprintf(outhdl, "_%d:", lbl1);
	gencode_I(TOK_PSHR, 0, 0);
	gencode_R(TOK_LDR, REG_AP, 1);

	// get arguments
	if (!match("("))
		error("no open parent");
	blanks();

	// now define arguments
	numarg = 0;
	while (ch != ')') {
		if (amatch("register"))
			declarg(scope, REGISTER, numarg++);
		else
			declarg(scope, AP_AUTO, numarg++);

		if (!match(","))
			break;
	}
	needtoken(")");

	// post-process arguments
	for (i = scope; i < symidx; i++) {
		sym = &syms[i * ILAST];

		// tweak ap offsets
		sym[IVALUE] += numarg * BPW;

		// generate code for register variables
		if (sym[ICLASS] == REGISTER) {
			int reg;
			reg = allocreg();
			reglock |= (1 << reg);
			gencode_IND(((sym[LSIZE] == BPW) || sym[LPTR]) ? TOK_LDW : TOK_LDB, reg, sym[IVALUE], REG_AP);
			sym[IVALUE] = reg;
		}
	}

	// get statement
	inireg = reglock;
	statement(swinx, returnlbl, 0, 0, csp, csp);
	if (csp != -1)
		error("internal error. stack not released");
	if (reglock != inireg)
		error("internal error. registers not unlocked");

	// trailing statements
	lbl2 = ++nxtlabel;
	fprintf(outhdl, "_%d:\t.ORG\t_%d\n", lbl2, lbl1);
	gencode_I(TOK_PSHR, 0, regsum);
	fprintf(outhdl, "\t.ORG\t_%d\n", lbl2);
	fprintf(outhdl, "_%d:", returnlbl);
	gencode_I(TOK_POPR, 0, regsum);
	gencode(TOK_RSB);

	symidx = scope;
}

/*
 * process one statement
 */
statement(int swbase, int returnlbl, int breaklbl, int contlbl, int breaksp, int contsp) {
	int lval[LLAST], scope, sav_sw;
	register int i, *sym;
	int lbl1, lbl2, lbl3;

	if (match("{")) {
		int last_csp, sav_csp;

		last_csp = sav_csp = csp;
		scope = symidx;
		while (!match("}")) {
			blanks();
			if (!ch) {
				error("no closing }"); // EOF
				return;
			} else if (amatch("register")) {
				declvar(scope, REGISTER);
			} else if (declvar(scope, SP_AUTO)) {
			} else {
				// allocate locals
				if (csp != last_csp) {
					gencode_ADJSP(csp - last_csp);
					last_csp = csp;
				}
				statement(swbase, returnlbl, breaklbl, contlbl, breaksp, contsp);
			}
		}
		// unwind stack
		if (csp != sav_csp) {
			gencode_ADJSP(sav_csp - csp);
			csp = sav_csp;
		}
		// free local registers
		for (i = scope; i < symidx; i++) {
			sym = &syms[i * ILAST];
			if (sym[ICLASS] == REGISTER)
				freereg(sym[IVALUE]);
		}
		symidx = scope;
		return;
	}

	if (amatch("if")) {
		needtoken("(");
		expression(lval, 1);
		needtoken(")");
		if (lval[LTYPE] == BRANCH) {
			if (!lval[LFALSE])
				lval[LFALSE] = ++nxtlabel;
			gencode_L(lval[LVALUE], lval[LFALSE]);
		} else {
			loadlval(lval, 0);
			lval[LFALSE] = ++nxtlabel;
			lval[LTRUE] = 0;
			gencode_L(TOK_BEQ, lval[LFALSE]);
		}
		if (lval[LTRUE])
			fprintf(outhdl, "_%d:", lval[LTRUE]);
		freelval(lval);
		statement(swbase, returnlbl, breaklbl, contlbl, breaksp, contsp);
		if (!amatch("else")) {
			// @date 2020-05-19 12:45:53
			//     compare
			// T:  statement
			// F:
			fprintf(outhdl, "_%d:", lval[LFALSE]);
		} else {
			// @date 2020-05-19 12:45:53
			//     compare
			// T:  statement
			//     jmp L1
			// F:  statement
			// L1:
			lbl1 = ++nxtlabel;
			gencode_L(TOK_JMP, lbl1);
			fprintf(outhdl, "_%d:", lval[LFALSE]);
			statement(swbase, returnlbl, breaklbl, contlbl, breaksp, contsp);
			fprintf(outhdl, "_%d:", lbl1);
		}
	} else if (amatch("while")) {
		// @date 2020-05-19 12:39:49
		// L1: compare
		//     bcc F
		// T:  statement
		//     jmp L1
		// F:
		lbl1 = ++nxtlabel;
		fprintf(outhdl, "_%d:", lbl1);
		needtoken("(");
		expression(lval, 1);
		needtoken(")");

		if (lval[LTYPE] == BRANCH) {
			if (!lval[LFALSE])
				lval[LFALSE] = ++nxtlabel;
			gencode_L(lval[LVALUE], lval[LFALSE]);
		} else {
			loadlval(lval, 0);
			lval[LFALSE] = ++nxtlabel;
			lval[LTRUE] = 0;
			gencode_L(TOK_BEQ, lval[LFALSE]);
		}
		if (lval[LTRUE])
			fprintf(outhdl, "_%d:", lval[LTRUE]);
		freelval(lval);
		statement(swbase, returnlbl, lval[LFALSE], lbl1, csp, csp);
		gencode_L(TOK_JMP, lbl1);
		fprintf(outhdl, "_%d:", lval[LFALSE]);
	} else if (amatch("do")) {
		// @date 2020-05-19 12:37:46
		// L1: statement
		// L2: compare
		//     bcc L1
		// F:
		lbl1 = ++nxtlabel;
		lbl2 = ++nxtlabel;
		fprintf(outhdl, "_%d:", lbl1);
		statement(swbase, returnlbl, lbl2, lbl1, csp, csp);
		fprintf(outhdl, "_%d:", lbl2);
		needtoken("while");
		needtoken("(");
		expression(lval, 1);
		needtoken(")");
		if (lval[LTYPE] == BRANCH) {
			if (lval[LTRUE])
				fprintf(outhdl, "_%d=_%d\n", lval[LTRUE], lbl1);
			else
				lval[LTRUE] = lbl1;
			gencode_L(negop(lval[LVALUE]), lval[LTRUE]);
			if (lval[LFALSE])
				fprintf(outhdl, "_%d:", lval[LFALSE]);
		} else {
			loadlval(lval, 0);
			gencode_L(TOK_BNE, lbl1);
		}
		freelval(lval);
	} else if (amatch("for")) {
		// @date 2020-05-19 12:37:25
		//     preamble
		// L1: compare
		//     bcc T
		// L2: increment
		//     jmp L1
		// T:  statement
		//     jmp L2
		// F:
		lbl1 = ++nxtlabel;
		lbl2 = ++nxtlabel;
		needtoken("(");
		blanks();
		if (ch != ';') {
			expression(lval, 1);
			freelval(lval);
		}
		needtoken(";");
		fprintf(outhdl, "_%d:", lbl1);
		blanks();
		if (ch != ';') {
			expression(lval, 1);
			if (lval[LTYPE] == BRANCH) {
				if (!lval[LFALSE])
					lval[LFALSE] = ++nxtlabel;
				if (!lval[LTRUE])
					lval[LTRUE] = ++nxtlabel;
				gencode_L(lval[LVALUE], lval[LFALSE]);
			} else {
				lval[LFALSE] = ++nxtlabel;
				lval[LTRUE] = ++nxtlabel;
				loadlval(lval, 0);
				gencode_L(TOK_BEQ, lval[LFALSE]);
			}
			freelval(lval);
		}
		gencode_L(TOK_JMP, lval[LTRUE]);
		needtoken(";");
		fprintf(outhdl, "_%d:", lbl2);
		blanks();
		if (ch != ')') {
			expression(lval, 1);
			freelval(lval);
		}
		gencode_L(TOK_JMP, lbl1);
		needtoken(")");
		fprintf(outhdl, "_%d:", lval[LTRUE]);
		statement(swbase, returnlbl, lval[LFALSE], lbl1, csp, csp);
		gencode_L(TOK_JMP, lbl2);
		fprintf(outhdl, "_%d:", lval[LFALSE]);
	} else if (amatch("switch")) {
		needtoken("(");
		expression(lval, 1);
		needtoken(")");
		loadlval(lval, 1);
		lbl1 = ++nxtlabel;
		lbl2 = ++nxtlabel;
		gencode_L(TOK_JMP, lbl1);
		sav_sw = swinx;
		if (swinx >= SWMAX)
			fatal("switch table overflow");
		sw[swinx++ * SLAST + SLABEL] = 0; // enable default
		statement(sav_sw, returnlbl, lbl2, contlbl, csp, contsp);
		gencode_L(TOK_JMP, lbl2);
		dumpsw(sav_sw, lbl1, lbl2);
		fprintf(outhdl, "_%d:", lbl2);
		swinx = sav_sw;
	} else if (amatch("case")) {
		if (!swbase)
			error("not in switch");
		if (!constexpr(&lbl3))
			error("case value expected");
		needtoken(":");
		for (i = swbase + 1; i < swinx; i++)
			if (sw[i * SLAST + SCASE] == lbl3)
				error("case value already defined");
		lbl1 = ++nxtlabel;
		fprintf(outhdl, "_%d:", lbl1);
		if (swinx >= SWMAX)
			fatal("switch table overflow");
		sym = &sw[swinx++ * SLAST];
		sym[SCASE] = lbl3;
		sym[SLABEL] = lbl1;
	} else if (amatch("default")) {
		if (!swbase)
			error("not in switch");
		needtoken(":");
		sym = &sw[swbase * SLAST];
		if (sym[SLABEL])
			error("multiple defaults");
		lbl1 = ++nxtlabel;
		fprintf(outhdl, "_%d:", lbl1);
		sym[SLABEL] = lbl1;
	} else if (amatch("return")) {
		if (!endst()) {
			// generate a return value in R1
			expression(lval, 1);
			loadlval(lval, 1);
		}
		if (csp != -1)
			gencode_ADJSP(-1 - csp);
		gencode_L(TOK_JMP, returnlbl);
		ns();
	} else if (amatch("break")) {
		if (!breaklbl)
			error("not in block");
		if (csp != breaksp)
			gencode_ADJSP(breaksp - csp);
		gencode_L(TOK_JMP, breaklbl);
		ns();
	} else if (amatch("continue")) {
		if (!contlbl)
			error("not in block");
		if (csp != contsp)
			gencode_ADJSP(contsp - csp);
		gencode_L(TOK_JMP, contlbl);
		ns();
	} else if (!ch) {
		return; // EOF
	} else if (ch != ';') {
		expression(lval, 1);
		freelval(lval);
		ns();
	} else
		ns();
}

/*
 * Dump the switch map and decoding
 */
dumpsw(int swbase, int codlbl, int endlbl) {
	register int lo, hi, i, j, cnt, *ptr;
	int maplbl, deflbl, lbl, lval[LLAST];

	if (swbase + 1 == swinx) {
		// no cases specified
		ptr = &sw[swbase * SLAST];
		if (ptr[SLABEL])
			gencode_L(TOK_JMP, ptr[SLABEL]);
		return;
	}

	// get lo/hi bounds
	ptr = &sw[(swbase + 1) * SLAST];
	lo = hi = ptr[SCASE];
	for (i = swbase + 1; i < swinx; i++) {
		ptr = &sw[i * SLAST];
		if (ptr[SCASE] > hi)
			hi = ptr[SCASE];
		if (ptr[SCASE] < lo)
			lo = ptr[SCASE];
	}

	// setup default
	if (!(deflbl = sw[swbase * SLAST + SLABEL]))
		deflbl = endlbl;

	// generate map
	maplbl = ++nxtlabel;
	toseg(DATASEG);
	fprintf(outhdl, "_%d:", maplbl);
	cnt = 0;
	for (i = lo; i <= hi; i++) {
		lbl = deflbl;
		for (j = swbase + 1; j < swinx; j++) {
			ptr = &sw[j * SLAST];
			if (ptr[SCASE] == i) {
				lbl = ptr[SLABEL];
				break;
			}
		}
		if (!cnt++)
			fprintf(outhdl, "\t.DCW\t_%d", lbl);
		else
			fprintf(outhdl, ",_%d", lbl);
		if (cnt > 15) {
			fprintf(outhdl, "\n");
			cnt = 0;
		}
	}
	if (cnt)
		fprintf(outhdl, "\n");
	toseg(prevseg);

	// generate code (use j as reg)
	fprintf(outhdl, "_%d:", codlbl);
	j = allocreg();
	gencode_I(TOK_LDA, j, lo);
	gencode_R(TOK_SUB, 1, j);
	gencode_L(TOK_BLT, deflbl);
	gencode_I(TOK_LDA, j, hi - lo);
	gencode_R(TOK_CMP, 1, j);
	gencode_L(TOK_BGT, deflbl);
	gencode_R(TOK_MUL, 1, REG_BPW);
	lval[LTYPE] = LABEL;
	lval[LNAME] = maplbl;
	lval[LREG1] = 1;
	lval[LREG2] = lval[LVALUE] = 0;
	gencode_M(TOK_LDW, j, lval);
	gencode_IND(TOK_JMP, 0, 0, j);
	freereg(j);
}

/*
 * process all input text
 *
 * At this level, only static declarations,
 *      defines, includes and function
 *      definitions are legal...
 */
parse() {
	blanks();
	while (inphdl) {
		if (ch == '{') {
			toseg(CODESEG);
			fprintf(outhdl, "_%d:", lastlbl); // constructor chain

			// initialise reserved registers on first call
			if (lastlbl == 1) {
				gencode_I(TOK_LDA, 13, BPW);
				gencode_I(TOK_LDA, 12, BPW * 2);
				gencode_I(TOK_LDA, 11, 1);
				gencode_I(TOK_LDA, 10, 0);
			}

			int returnlbl;

			returnlbl = ++nxtlabel;
			reguse = regsum = reglock = 1 << REG_AP; // reset all registers
			csp = -1; // reset stack
			swinx = 1;

			// get statement
			statement(swinx, returnlbl, 0, 0, csp, csp);
			if (csp != -1)
				error("internal error. stack not released");

			fprintf(outhdl, "_%d:", returnlbl);
			lastlbl = ++nxtlabel;
			gencode_L(TOK_JMP, lastlbl); // forward reference to constructor chain

		} else if (amatch("enum"))
			declenum();
		else if (amatch("extern"))
			declvar(0, EXTERNAL);
		else if (amatch("static"))
			declvar(0, STATIC);
		else if (amatch("register")) {
			error("global register variables not allowed");
			declvar(0, GLOBAL);
		} else if (declvar(0, GLOBAL))
			;
		else if (amatch("#include"))
			doinclude();
		else if (amatch("#define"))
			declmac();
		else
			declfunc();
		blanks();
	}
}

