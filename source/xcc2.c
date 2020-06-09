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
declarg(int scope, register int clas, register int argnr) {
	int size, sname, len, ptr, type, reg, ptrfunc;
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

		ptrfunc = 0;
		if (match("*"))
			ptr = 1;
		else if (match("(")) {
			ptrfunc = 1;
			needtoken("*");
		} else
			ptr = 0;
		blanks();

		if (!(len = dohash(lptr, &sname)))
			illname();
		if (len)
			bump(len);

		for (i = scope; i < symidx; ++i) {
			sym = &syms[i * ILAST];
			if (sym[ISYM] == sname) {
				multidef();
				break;
			}
		}

		type = MEMORY;

		if (ptrfunc) {
			needtoken(")");
			if (match("(")) {
				if (match(")")) {
					type = FUNCTION;
					ptr = 1;
					if (clas == REGISTER)
						error("register pointer to function not supported");
				} else
					error("bad (*)()");
			}
		}

		if (match("[")) {
			if (ptr)
				error("array of pointers not supported");
			if (type != MEMORY)
				error("array type not supported");

			ptr = 1; // address of array (passed as argument) is pushed on stack

			// get number of elements
			int cnt;
			if (constexpr(&cnt))
				error("arraysize not allowed");

			// force single dimension
			needtoken("]");
		}

		// add symbol to symboltable
		if (symidx >= SYMMAX)
			fatal("identifier table overflow");
		sym = &syms[symidx++ * ILAST];
		sym[ISYM] = sname;
		sym[ICLASS] = clas;
		sym[ITYPE] = type;
		sym[IPTR] = ptr;
		sym[ISIZE] = size;
		sym[INAME] = 0;
		sym[IVALUE] = (-argnr + 1) * BPW;
		sym[IREG] = REG_AP;

		// modify location if chars. Chars are pushed as words, adjust to point to lo-byte
		if (size == 1 && !ptr)
			sym[IVALUE] += BPW - 1;

		dump_ident(sym);

		// only one
		break;
	}
}

/*
 * General global definitions
 */
declvar(int scope, register int clas) {
	int size, sname, len, ptr, type, cnt, ptrfunc;
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

		ptrfunc = 0;
		if (match("*"))
			ptr = 1;
		else if (match("(")) {
			ptrfunc = 1;
			needtoken("*");
		} else
			ptr = 0;
		blanks();

		if (!(len = dohash(lptr, &sname)))
			illname();
		if (len)
			bump(len);

		for (i = scope; i < symidx; ++i) {
			if (syms[i * ILAST + ISYM] == sname) {
				multidef();
				break;
			}
		}

		type = MEMORY;

		if (ptrfunc) {
			needtoken(")");
			if (match("(")) {
				if (match(")")) {
					type = FUNCTION;
					ptr = 1;
					if (clas == REGISTER)
						error("register pointer to function not supported");
				} else
					error("bad (*)()");
			}
		}

		cnt = 1; // Number of elements
		if (match("[")) {
			if (clas == REGISTER)
				error("register array not supported");

			type = ADDRESS;
			// add extra indirection to endtype
			++ptr;

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
		sym[ISYM] = sname;
		sym[ICLASS] = clas;
		sym[ITYPE] = type;
		sym[IPTR] = ptr;
		sym[ISIZE] = size;
		sym[INAME] = sname;
		sym[IVALUE] = 0;
		sym[IREG] = 0;

		// Now generate code
		if (sym[ICLASS] == REGISTER) {
			sym[ITYPE] = ADDRESS;
			sym[INAME] = 0;
			sym[IVALUE] = 0;
			sym[IREG] = allocreg();
			reglock |= (1 << sym[IREG]);
		} else if (sym[ICLASS] == SP_AUTO) {

			if (type == ADDRESS) {
				if (ptr <= 1 && size == 1)
					csp -= cnt * 1;
				else
					csp -= cnt * BPW;
			} else {
				if (!ptr && size == 1)
					csp -= cnt * 1;
				else
					csp -= cnt * BPW;
			}

			sym[INAME] = 0;
			sym[IVALUE] = csp;
			sym[IREG] = REG_SP;
		} else if (sym[ICLASS] != EXTERNAL) {
			toseg(UDEFSEG);
			fprintf(outhdl, "_");
			symname(sname);
			fprintf(outhdl, ":");

			if (clas != STATIC)
				fprintf(outhdl, ":");

			if (type == ADDRESS) {
				if (ptr <= 1 && size == 1)
					fprintf(outhdl, "\t.DSB\t%d\n", cnt);
				else
					fprintf(outhdl, "\t.DSW\t%d\n", cnt);
			} else {
				if (!ptr && size == 1)
					fprintf(outhdl, "\t.DSB\t1\n");
				else
					fprintf(outhdl, "\t.DSW\t1\n");
			}
		}
		dump_ident(sym);

		// test for more
		if (!match(","))
			break;
	}

	// done
	semicolon();
	return 1;
}

/*
 *
 */
declenum(int scope) {
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
			semicolon();
			return;
		}
		bump(len);

		// add symbol to symboltable
		for (i = scope; i < symidx; ++i) {
			sym = &syms[i * ILAST];
			if (sym[ISYM] == sname) {
				multidef();
				break;
			}
		}

		if (symidx >= SYMMAX)
			fatal("identifier table overflow");
		sym = &syms[symidx++ * ILAST];
		sym[ISYM] = sname;
		sym[ICLASS] = EXTERNAL; // external has no storage
		sym[ITYPE] = ADDRESS;
		sym[IPTR] = 0;
		sym[ISIZE] = 0;
		sym[INAME] = 0;
		sym[IVALUE] = 0;
		sym[IREG] = 0;

		if (match("=")) {
			if (!constexpr(&seqnr))
				expected("constant expected");
		}

		sym[IVALUE] = seqnr++;
		dump_ident(sym);

		if (!match(","))
			break;
	}
	needtoken("}");
	semicolon();
}

/*
 * open an include file
 */
doinclude() {
	register char *p;

	white();
	if (*lptr != '"')
		expected("filename");
	else if (inchdl)
		error("Nested #include not allowed");
	else {
		// Modify sourceline to extract filename
		p = incfn;
		gch(); // Skip delimiter
		while ((*p++ = gch()) != '"');
		*--p = 0; // Remove delimiter

		// Open file
		inchdl = open_file(incfn, "r");
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
		expected("identifier");
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
declfunc(int clas) {
	int returnlbl, len, sname, lbl1, lbl2, inireg, sav_argc, scope;
	register int *sym, i, numarg;

	returnlbl = ++nxtlabel;
	reguse = regsum = 1 << REG_AP; // reset all registers
	reglock = regresvd | 1 << REG_AP; // locked registers include reserved registers
	csp = 0; // reset stack
	swinx = 1;
	toseg(CODESEG);
	if (verbose)
		printf("%s\n", lptr);

	// get procedure name
	if (!(len = dohash(lptr, &sname))) {
		fatal("illegal function name or declaration");
		return;
	}
	bump(len);

	scope = symidx;

	for (i = 0; i < symidx; ++i) {
		sym = &syms[i * ILAST];
		if (sym[ISYM] == sname)
			break;
	}

	if (symidx >= SYMMAX)
		fatal("identifier table overflow");
	if (i >= symidx)
		sym = &syms[symidx++ * ILAST];

	// (re)define function
	sym[ISYM] = sname;
	sym[ICLASS] = EXTERNAL;
	sym[ITYPE] = FUNCTION;
	sym[IPTR] = 0;
	sym[ISIZE] = 0;
	sym[INAME] = sname;
	sym[IVALUE] = 0;
	sym[IREG] = 0;
	dump_ident(sym);

	// Generate global label
	fprintf(outhdl, "_");
	symname(sname);
	fprintf(outhdl, "::");
	gencode_R(TOK_LDR, REG_RETURN, REG_SP);
	lbl1 = ++nxtlabel;
	fprintf(outhdl, "_%d:", lbl1);
	gencode_I(TOK_PSHR, -1, 0);
	gencode_R(TOK_LDR, REG_AP, 1);

	// get parameters
	needtoken("(");
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

	if (match(";")) {
		// declaration only
		symidx = scope;
		return;
	}

	if (sym[ICLASS] != EXTERNAL)
		multidef();
	sym[ICLASS] = clas;

	// post-process parameters. syms[scope] is function name
	for (i = scope + 1; i < symidx; ++i) {
		sym = &syms[i * ILAST];

		// tweak AP offsets
		sym[IVALUE] += numarg * BPW;

		// generate code for register parameters
		if (sym[ICLASS] == REGISTER) {
			int reg;
			reg = allocreg();
			reglock |= (1 << reg);
			gencode_M((sym[ISIZE] == BPW || sym[IPTR]) ? TOK_LDW : TOK_LDB, reg, sym[INAME], sym[IVALUE], sym[IREG]);
			sym[ITYPE] = ADDRESS;
			sym[INAME] = 0;
			sym[IVALUE] = 0;
			sym[IREG] = reg;
			dump_ident(sym);
		}
	}

	// get statement
	inireg = reglock;
	statement(swinx, returnlbl, 0, 0, csp, csp);
	if (csp != 0)
		error("internal error. stack not released");
	if (inireg != reglock)
		error("internal error. registers not unlocked");

	// trailing statements
	lbl2 = ++nxtlabel;
	fprintf(outhdl, "_%d:\t.ORG\t_%d\n", lbl2, lbl1);
	gencode_I(TOK_PSHR, -1, regsum);
	fprintf(outhdl, "\t.ORG\t_%d\n", lbl2);
	fprintf(outhdl, "_%d:", returnlbl);
	gencode_I(TOK_POPR, -1, regsum);
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
				expected("}"); // EOF
				return;
			} else if (amatch("enum")) {
				declenum(scope);
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
		for (i = scope; i < symidx; ++i) {
			sym = &syms[i * ILAST];
			if (sym[ICLASS] == REGISTER)
				freereg(sym[IREG]);
		}
		symidx = scope;
		return;
	}

	if (amatch("if")) {
		needtoken("(");
		expression(lval);
		needtoken(")");
		if (lval[LTYPE] == BRANCH) {
			if (!lval[LFALSE])
				lval[LFALSE] = ++nxtlabel;
			gencode_L(lval[LVALUE], lval[LFALSE]);
		} else {
			loadlval(lval, 0);
			lval[LTRUE] = 0;
			lval[LFALSE] = ++nxtlabel;
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
		expression(lval);
		needtoken(")");

		if (lval[LTYPE] == BRANCH) {
			if (!lval[LFALSE])
				lval[LFALSE] = ++nxtlabel;
			gencode_L(lval[LVALUE], lval[LFALSE]);
		} else {
			loadlval(lval, 0);
			lval[LTRUE] = 0;
			lval[LFALSE] = ++nxtlabel;
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
		expression(lval);
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
			expression(lval);
			freelval(lval);
		}
		needtoken(";");
		fprintf(outhdl, "_%d:", lbl1);
		blanks();
		if (ch != ';') {
			expression(lval);
			if (lval[LTYPE] == BRANCH) {
				if (!lval[LFALSE])
					lval[LFALSE] = ++nxtlabel;
				if (!lval[LTRUE])
					lval[LTRUE] = ++nxtlabel;
				gencode_L(lval[LVALUE], lval[LFALSE]);
			} else {
				lval[LTRUE] = ++nxtlabel;
				lval[LFALSE] = ++nxtlabel;
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
			expression(lval);
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
		expression(lval);
		needtoken(")");
		loadlval(lval, REG_RETURN);
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
			expected("case value");
		needtoken(":");
		for (i = swbase + 1; i < swinx; ++i)
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
		if (!match(";")) {
			// generate a return value in R1
			expression(lval);
			loadlval(lval, REG_RETURN);
			semicolon();
		}
		if (csp != 0)
			gencode_ADJSP(- csp);
		gencode_L(TOK_JMP, returnlbl);
	} else if (amatch("break")) {
		if (!breaklbl)
			error("not in block");
		if (csp != breaksp)
			gencode_ADJSP(breaksp - csp);
		gencode_L(TOK_JMP, breaklbl);
		semicolon();
	} else if (amatch("continue")) {
		if (!contlbl)
			error("not in block");
		if (csp != contsp)
			gencode_ADJSP(contsp - csp);
		gencode_L(TOK_JMP, contlbl);
		semicolon();
	} else if (!ch) {
		return; // EOF
	} else if (ch != ';') {
		expression(lval);
		freelval(lval);
		semicolon();
	} else
		semicolon();
}

/*
 * Dump the switch map and decoding
 */
dumpsw(int swbase, int codlbl, int endlbl) {
	register int lo, hi, i, j, cnt, *ptr;
	int maplbl, deflbl, lbl;

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
	for (i = swbase + 1; i < swinx; ++i) {
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
	for (i = lo; i <= hi; ++i) {
		lbl = deflbl;
		for (j = swbase + 1; j < swinx; ++j) {
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
	gencode_R(TOK_SUB, REG_RETURN, j);
	gencode_L(TOK_BLT, deflbl);
	gencode_I(TOK_LDA, j, hi - lo);
	gencode_R(TOK_CMP, REG_RETURN, j);
	gencode_L(TOK_BGT, deflbl);
	gencode_R(TOK_MUL, REG_RETURN, REG_BPW);
	gencode_M(TOK_LDW, j, -maplbl, 0, REG_RETURN);
	gencode_M(TOK_JMP, -1, 0, 0, j);
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
		if (amatch("enum"))
			declenum(0);
		else if (amatch("#include"))
			doinclude();
		else if (amatch("#define"))
			declmac();
		else {
			int clas;
			if (amatch("extern"))
				clas = EXTERNAL;
			else if (amatch("static"))
				clas = STATIC;
			else
				clas = GLOBAL;

			if (amatch("register")) {
				reglock = 0;
				if (declvar(0, REGISTER))
					regresvd |= reglock;
			} else if (declvar(0, clas)) {
				;
			} else {
				declfunc(clas);
			}
		}
		blanks();
	}
}

