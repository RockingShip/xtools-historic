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
	register int *ident, i;

	// get size of type
	if (amatch("char"))
		size = 1; // char is stored as int on stack
	else if (amatch("int"))
		size = 2;
	else
		return 0;

	// scan definitions
	while (1) {
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

		for (i = scope; i < idinx; i++) {
			ident = &idents[i * ILAST];
			if (ident[INAME] == sname) {
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
		if (idinx >= IDMAX)
			fatal("identifier table overflow");
		ident = &idents[idinx++ * ILAST];
		ident[INAME] = sname;
		ident[ITYPE] = type;
		ident[IPTR] = ptr;
		ident[ICLASS] = class;
		ident[ISIZE] = size;
		ident[IVALUE] = (-argnr + 1) * BPW;

		// modify location if chars. Chars are pushed as words, adjust to point to lo-byte
		if (size == 1 && !ptr)
			ident[IVALUE] += BPW - 1;

		// only one
		break;
	}
}

/*
 * General global definitions
 */
declvar(int scope, register int class) {
	int size, sname, len, ptr, type, cnt;
	register int *ident, i;

	// get size of type
	if (amatch("char"))
		size = 1;
	else if (amatch("int"))
		size = 2;
	else
		return 0;

	// scan definitions
	while (1) {
		ptr = (match("(*") || match("*"));
		if (!(len = dohash(lptr, &sname)))
			illname();
		if (len)
			bump(len);

		for (i = scope; i < idinx; i++) {
			if (idents[i * ILAST + INAME] == sname)
				break;
		}
		if (i < idinx)
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
		if (idinx >= IDMAX)
			fatal("identifier table overflow");
		ident = &idents[idinx++ * ILAST];
		ident[INAME] = sname;
		ident[ITYPE] = type;
		ident[IPTR] = ptr;
		ident[ICLASS] = class;
		ident[ISIZE] = size;
		ident[IVALUE] = 0;

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

		} else if (ident[ICLASS] == REGISTER) {
			ident[IVALUE] = allocreg();
			reglock |= (1 << ident[IVALUE]);

		} else if (ident[ICLASS] == SP_AUTO) {
			if (ptr)
				ident[IVALUE] = (csp -= BPW);
			else if (size == 1)
				ident[IVALUE] = (csp -= cnt);
			else
				ident[IVALUE] = (csp -= cnt * BPW);

		} else if (ident[ICLASS] != EXTERNAL) {
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
	int *s;
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
		for (i = idinx - 1; i >= 0; i--) {
			s = &idents[i * ILAST];
			if (s[INAME] == sname)
				break;
		}
		if (i >= 0)
			multidef();
		else if (idinx >= IDMAX)
			fatal("identifier table overflow");
		s = &idents[idinx++ * ILAST];

		s[INAME] = sname;
		s[ITYPE] = CONSTANT; // todo: this should actually be VARIABLE
		s[IPTR] = 0;
		s[ICLASS] = 0; // todo: this should actually be CONSTANT
		s[IVALUE] = 0;
		s[ISIZE] = 0;

		if (match("=")) {
			expression(lval, 0);
			if (lval[LTYPE] != CONSTANT) {
				error("expected expression");
				return;
			}
			seqnr = lval[LVALUE];
		}

		s[IVALUE] = seqnr++;

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
 * Copy assembler source directly to the output file
 */
doasm() {
	ccode = 0; // mark mode as "asm"
	readline(); // skip #asm
	while (inphdl) {
		white();
		if (ch && amatch("#endasm"))
			break;
		fprintf(outhdl, "%s\n", line);
		readline();
	}
	kill(); // erase to eoln
	ccode = 1;
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
	register int *ident, i, numarg;

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

	scope = idinx;

	for (i = 0; i < idinx; i++) {
		ident = &idents[i * ILAST];
		if (ident[INAME] == sname)
			break;
	}
	if (i < idinx && ident[ICLASS] != AUTOEXT)
		multidef();
	if (idinx >= IDMAX)
		fatal("identifier table overflow");

	if (i >= idinx)
		ident = &idents[idinx++ * ILAST];

	// (re)define procedure
	ident[INAME] = sname;
	ident[ITYPE] = FUNCTION;
	ident[IPTR] = 0;
	ident[ICLASS] = GLOBAL;
	ident[IVALUE] = 0;
	ident[ISIZE] = BPW;
	// Generate global label
	fprintf(outhdl, "_");
	symname(sname);
	fprintf(outhdl, "::");
	gencode_R(OPC_LDR, 1, REG_SP);
	lbl1 = ++nxtlabel;
	fprintf(outhdl, "_%d:", lbl1);
	gencode_I(OPC_PSHR, 0, 0);
	gencode_R(OPC_LDR, REG_AP, 1);

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
	for (i = scope; i < idinx; i++) {
		ident = &idents[i * ILAST];

		// tweak ap offsets
		ident[IVALUE] += numarg * BPW;

		// generate code for register variables
		if (ident[ICLASS] == REGISTER) {
			int reg;
			reg = allocreg();
			reglock |= (1 << reg);
			gencode_IND(((ident[LSIZE] == BPW) || ident[LPTR]) ? OPC_LDW : OPC_LDB, reg, ident[IVALUE], REG_AP);
			ident[IVALUE] = reg;
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
	gencode_I(OPC_PSHR, 0, regsum);
	fprintf(outhdl, "\t.ORG\t_%d\n", lbl2);
	fprintf(outhdl, "_%d:", returnlbl);
	gencode_I(OPC_POPR, 0, regsum);
	gencode(OPC_RSB);

	idinx = scope;
}

/*
 * process one statement
 */
statement(int swbase, int returnlbl, int breaklbl, int contlbl, int breaksp, int contsp) {
	int lval[LLAST], scope, sav_sw;
	register int sav_csp, i, *ptr;
	int lbl1, lbl2, lbl3;

	if (match("{")) {
		sav_csp = csp;
		scope = idinx;
		while (!match("}")) {
			if (ch <= ' ')
				blanks();
			if (!ch) {
				error("no closing }"); // EOF
				return;
			} else if (amatch("register")) {
				declvar(scope, REGISTER);
				if (sav_csp > 0)
					error("Definitions not allowed here");
			} else if (declvar(scope, SP_AUTO)) {
				if (sav_csp > 0)
					error("Definitions not allowed here");
			} else {
				// allocate locals
				if ((sav_csp < 0) && (csp != sav_csp)) {
					gencode_ADJSP(csp - sav_csp);
					sav_csp = -sav_csp;
				}
				statement(swbase, returnlbl, breaklbl, contlbl, breaksp, contsp);
			}
		}
		// done
		if (sav_csp > 0)
			sav_csp = -sav_csp;
		if (csp != sav_csp) {
			gencode_ADJSP(sav_csp - csp);
			csp = sav_csp;
		}
		// free local registers
		for (i = scope; i < idinx; i++) {
			ptr = &idents[i * ILAST];
			if (ptr[ICLASS] == REGISTER)
				freereg(ptr[IVALUE]);
		}
		idinx = scope;
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
			gencode_L(OPC_EQ, lval[LFALSE]);
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
			gencode_L(OPC_JMP, lbl1);
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
			gencode_L(OPC_EQ, lval[LFALSE]);
		}
		if (lval[LTRUE])
			fprintf(outhdl, "_%d:", lval[LTRUE]);
		freelval(lval);
		statement(swbase, returnlbl, lval[LFALSE], lbl1, csp, csp);
		gencode_L(OPC_JMP, lbl1);
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
			gencode_L(OPC_NE, lbl1);
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
		if (ch <= ' ')
			blanks();
		if (ch != ';') {
			expression(lval, 1);
			freelval(lval);
		}
		needtoken(";");
		fprintf(outhdl, "_%d:", lbl1);
		if (ch <= ' ')
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
				gencode_L(OPC_EQ, lval[LFALSE]);
			}
			freelval(lval);
		}
		gencode_L(OPC_JMP, lval[LTRUE]);
		needtoken(";");
		fprintf(outhdl, "_%d:", lbl2);
		if (ch <= ' ')
			blanks();
		if (ch != ')') {
			expression(lval, 1);
			freelval(lval);
		}
		gencode_L(OPC_JMP, lbl1);
		needtoken(")");
		fprintf(outhdl, "_%d:", lval[LTRUE]);
		statement(swbase, returnlbl, lval[LFALSE], lbl1, csp, csp);
		gencode_L(OPC_JMP, lbl2);
		fprintf(outhdl, "_%d:", lval[LFALSE]);
	} else if (amatch("switch")) {
		needtoken("(");
		expression(lval, 1);
		needtoken(")");
		loadlval(lval, 1);
		lbl1 = ++nxtlabel;
		lbl2 = ++nxtlabel;
		gencode_L(OPC_JMP, lbl1);
		sav_sw = swinx;
		if (swinx >= SWMAX)
			fatal("switch table overflow");
		sw[swinx++ * SLAST + SLABEL] = 0; // enable default
		statement(sav_sw, returnlbl, lbl2, contlbl, csp, contsp);
		gencode_L(OPC_JMP, lbl2);
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
		ptr = &sw[swinx++ * SLAST];
		ptr[SCASE] = lbl3;
		ptr[SLABEL] = lbl1;
	} else if (amatch("default")) {
		if (!swbase)
			error("not in switch");
		needtoken(":");
		ptr = &sw[swbase * SLAST];
		if (ptr[SLABEL])
			error("multiple defaults");
		lbl1 = ++nxtlabel;
		fprintf(outhdl, "_%d:", lbl1);
		ptr[SLABEL] = lbl1;
	} else if (amatch("return")) {
		if (!endst()) {
			// generate a return value in R1
			expression(lval, 1);
			loadlval(lval, 1);
		}
		if (csp != -1)
			gencode_ADJSP(-1 - csp);
		gencode_L(OPC_JMP, returnlbl);
		ns();
	} else if (amatch("break")) {
		if (!breaklbl)
			error("not in block");
		if (csp != breaksp)
			gencode_ADJSP(breaksp - csp);
		gencode_L(OPC_JMP, breaklbl);
		ns();
	} else if (amatch("continue")) {
		if (!contlbl)
			error("not in block");
		if (csp != contsp)
			gencode_ADJSP(contsp - csp);
		gencode_L(OPC_JMP, contlbl);
		ns();
	} else if (!ch) {
		return; // EOF
	} else if (amatch("#asm")) {
		doasm();
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
			gencode_L(OPC_JMP, ptr[SLABEL]);
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
	gencode_I(OPC_LEA, j, lo);
	gencode_R(OPC_SUB, 1, j);
	gencode_L(OPC_LT, deflbl);
	gencode_I(OPC_LEA, j, hi - lo);
	gencode_R(OPC_CMP, 1, j);
	gencode_L(OPC_GT, deflbl);
	gencode_R(OPC_MUL, 1, REG_BPW);
	lval[LTYPE] = LABEL;
	lval[LNAME] = maplbl;
	lval[LREG1] = 1;
	lval[LREG2] = lval[LVALUE] = 0;
	gencode_M(OPC_LDW, j, lval);
	gencode_IND(OPC_JMP, 0, 0, j);
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
			declenum();
		else if (amatch("extern"))
			declvar(0, EXTERNAL);
		else if (amatch("static"))
			declvar(0, STATIC);
		else if (amatch("register")) {
			error("global register variables not allowed");
			declvar(0, GLOBAL);
		} else if (declvar(0, GLOBAL));
		else if (amatch("#asm"))
			doasm();
		else if (amatch("#include"))
			doinclude();
		else if (amatch("#define"))
			declmac();
		else
			declfunc();
		blanks();
	}
}

