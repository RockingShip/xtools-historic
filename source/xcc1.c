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
//* X-C-Compiler.  Part 1, I/O Support routines
//*

#define EXTERN
#include "xcc.h"

//*
//*
//* Compiler startup routines
//*
//*

/*
 * Initialize all variables
 */
initialize() {
	register int i;

	verbose = maklis = 0;
	outhdl = lishdl = inphdl = inchdl = 0;
	iflevel = skiplevel = 0;
	nxtlabel = 0;
	currseg = 0;
	inpfn[0] = 0;
	macinx = macqinx = 0;
	ccode = 1;
	inclnr = inplnr = 0;

	// character properties
	for (i = '0'; i <= '9'; i++)
		ctype[i] = CISDIGIT | CISXDIGIT | CSYMNEXT;
	for (i = 'A'; i <= 'F'; i++)
		ctype[i] = CISUPPER | CISXDIGIT | CSYMFIRST | CSYMNEXT;
	for (i = 'G'; i <= 'Z'; i++)
		ctype[i] = CISUPPER | CSYMFIRST | CSYMNEXT;
	for (i = 'a'; i <= 'f'; i++)
		ctype[i] = CISLOWER | CISXDIGIT | CSYMFIRST | CSYMNEXT;
	for (i = 'g'; i <= 'z'; i++)
		ctype[i] = CISLOWER | CSYMFIRST | CSYMNEXT;
	ctype['_'] = CSYMFIRST | CSYMNEXT;
	ctype[' '] = CISSPACE;
	ctype['\f'] = CISSPACE;
	ctype['\n'] = CISSPACE;
	ctype['\r'] = CISSPACE;
	ctype['\t'] = CISSPACE;
	ctype['\v'] = CISSPACE;

	// reset table
	for (i = 0; i < NAMEMAX; i++)
		namech[i] = nametab[i] = 0;
	for (i = 0; i < SYMMAX; i++)
		syms[i * ILAST + INAME] = 0;
	for (i = 0; i < SWMAX; i++)
		sw[i * SLAST + SLABEL] = 0;

	// reserve first entry so it terminates lists
	namech[0] = '?';

	// setup array containing hieriachal operators
	hier_str[ 0] = "||"; hier_oper[ 0] = TOK_OR;    // hier3
	hier_str[ 1] = 0;
	hier_str[ 2] = "&&"; hier_oper[ 2] = TOK_AND;   // hier4
	hier_str[ 3] = 0;
	hier_str[ 4] = "|";  hier_oper[ 4] = TOK_OR;    // hier5
	hier_str[ 5] = 0;
	hier_str[ 6] = "^";  hier_oper[ 6] = TOK_XOR;    // hier6
	hier_str[ 7] = 0;
	hier_str[ 8] = "&";  hier_oper[ 8] = TOK_AND;   // hier7
	hier_str[ 9] = 0;
	hier_str[10] = "=="; hier_oper[10] = TOK_BEQ;     // hier8
	hier_str[11] = "!="; hier_oper[11] = TOK_BNE;
	hier_str[12] = 0;
	hier_str[13] = "<="; hier_oper[13] = TOK_BLE;     // hier9
	hier_str[14] = ">="; hier_oper[14] = TOK_BGE;
	hier_str[15] = "<";  hier_oper[15] = TOK_BLT;
	hier_str[16] = ">";  hier_oper[16] = TOK_BGT;
	hier_str[17] = 0;
	hier_str[18] = ">>"; hier_oper[18] = TOK_LSR;    // hier10
	hier_str[19] = "<<"; hier_oper[19] = TOK_LSL;
	hier_str[20] = 0;
	hier_str[21] = "+";  hier_oper[21] = TOK_ADD;    // hier11
	hier_str[22] = "-";  hier_oper[22] = TOK_SUB;
	hier_str[23] = 0;
	hier_str[24] = "*";  hier_oper[24] = TOK_MUL;    // hier12
	hier_str[25] = "/";  hier_oper[25] = TOK_DIV;
	hier_str[26] = "%";  hier_oper[26] = TOK_MOD;
	hier_str[27] = 0;

	// reserved words
	dohash("ARGC", &argcid);
	dohash("ARGV", &argvid);
}

/*
 * Process commandline
 */
usage() {
	printf("X-C-Compiler, Version %s\n\n", getversion());

	printf("usage: xcc <file>[.<ext>]\n");
	printf("  -h\t\t\tInclude high-level source\n");
	printf("  -S <file>[.<ext>]]\tAssembler output\n");
	printf("  -v\t\t\tVerbose\n");
	exit(1);
}

/*
 * Override default/explicit file extension
 */
fext(char *out, char *path, char *ext, int force) {
	char *p;
	int baselen;

	baselen = 0;
	for (p = path; *p; p++) {
		if (*p == '\\' || *p == '/')
			baselen = 0;
		else if (*p == '.')
			baselen = p - path;
	}

	if (baselen && !force)
		strcpy(out, path);
	else {
		if (!baselen)
			baselen = p - path;
		strncpy(out, path, baselen);
		strcpy(out + baselen, ext);
	}
}

/*
 * Handle program arguments
 */
startup(register int *argv) {
	argv++; // skip argv[0]
	while (*argv) {
		register char *arg;
		arg = *argv++;

		if (*arg != '-') {
			fext(inpfn, arg, ".c", 0);
			if (!outfn[0])
				fext(outfn, arg, ".xs", 1);
		} else {
			// Process option
			arg++;
			switch (*arg++) {
			case 'S':
				if (!*arg && *argv)
					arg = *argv++;
				if (!*arg || *arg == '-')
					usage();
				else
					fext(outfn, arg, ".xs", 0);
				break;
			case 'h':
				maklis = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				usage();
				break;
			}
		}
	}

	// filename MUST be supplied
	if (!outfn[0])
		usage();
}

/*
 * Open all files
 */
mustopen(char *fn, char *mode) {
	int fd;

	fd = fopen(fn, mode);
	if (fd > 0)
		return fd;
	printf("fopen(%s,%s) failed\n", fn, mode);
	exit(1);
}

openfile() {
	inphdl = mustopen(inpfn, "r");
	outhdl = mustopen(outfn, "w");
}

//*
//*
//* Input support routines
//*
//*

/*
 * Get next character from current line
 */
gch() {
	register int c;

	if (c = ch)
		bump(1);
	return c;
}

/*
 * Erase current line
 */
kill() {
	*line = 0;
	bump(0);
}

/*
 * Bump next characters in line (n = #chars or 0 for initialization)
 */
bump(register int n) {
	lptr = n ? lptr + n : line;
	nch = (ch = lptr[0]) ? lptr[1] : 0;
}

//*
//*
//*  Symboltable routines
//*
//*

/*
 * reconstruct the symbol name
 */
symname(register int tab) {
	register int i;

	i = nametab[tab];
	if (i)
		symname(i);
	fprintf(outhdl, "%c", namech[tab]);
}

/*
 * Get the (unique) hashed value for symbol, return length
 */
dohash(register char *name, int *retval) {
	register int start, hash, tab, len;

	if (~ctype[*name] & CSYMFIRST)
		return 0; // Not a symbol

	tab = 0;
	len = 0;
	hash = 0;
	while (ctype[*name] & CSYMNEXT) {
		start = hash = (hash + *name * *name) % NAMEMAX;
		while (1) {
			if ((namech[hash] == *name) && (nametab[hash] == tab)) {
				tab = hash;
				break; // Inner loop
			} else if (!namech[hash]) {
				namech[hash] = *name;
				nametab[hash] = tab;
				tab = hash;
				break; // Inner loop
			} else {
				hash += *name;
				if (hash >= NAMEMAX)
					hash -= NAMEMAX;
				if (hash == start)
					fatal("name table overflow");
			}
		}
		++name;
		++len;
	}
	*retval = tab;
	return len;
}

findmac(register int sname) {
	register int i;
	register int *mptr;

	for (i = 0; i < macinx; i++) {
		mptr = &mac[i * MLAST];
		if (mptr[MNAME] == sname)
			return mptr;
	}
	return 0;
}

//*
//*
//* Preprocessor
//*
//*

keepch(int c) {
	if (pinx < PBUFMAX)
		pbuf[pinx++] = c;
}

readline() {
	sbuf[0] = 0;
	if (inphdl)
		while (!sbuf[0]) {
			if (inchdl) {
				if (!fgets(sbuf, SBUFMAX - 1, inchdl)) {
					fclose(inchdl);
					inchdl = 0;
					continue;
				}
				sbuf[SBUFMAX - 1] = 0;
				++inclnr;
			} else if (inphdl) {
				if (!fgets(sbuf, SBUFMAX - 1, inphdl)) {
					fclose(inphdl);
					inphdl = 0;
					break;
				}
				sbuf[SBUFMAX - 1] = 0;
				++inplnr;
			}
		}

	// Make buffer available
	line = sbuf;
	bump(0);
}

ifline() {
	int sname;

	while (1) {
		readline();
		if (!inphdl)
			break;

		// Skip blanks manually here, otherwise amatch() will call blanks() and this will cause recursion
		white();
		if (!ch)
			continue; // Try again

		if (amatch("#ifdef")) {
			++iflevel;
			if (!skiplevel) {
				white();
				if (!dohash(lptr, &sname))
					error("identifier expected");
				else if (!findmac(sname))
					skiplevel = iflevel;
			}
		} else if (amatch("#ifndef")) {
			++iflevel;
			if (!skiplevel) {
				white();
				if (!dohash(lptr, &sname))
					error("identifier expected");
				else if (findmac(sname))
					skiplevel = iflevel;
			}
		} else if (amatch("#else")) {
			if (iflevel) {
				if (skiplevel == iflevel)
					skiplevel = 0;
				else if (!skiplevel)
					skiplevel = iflevel;
			} else
				error("no matching #if...");
		} else if (amatch("#endif")) {
			if (iflevel) {
				if (skiplevel == iflevel)
					skiplevel = 0;
				--iflevel;
			} else
				error("no matching #if...");
		} else if (!skiplevel)
			return 0; // Process this line
	}
}

preprocess() {
	register char *optr, *cptr;
	register int i, len;
	int *mptr;
	int sname;

	if (ccode) {
		ifline();
		if (!inphdl)
			return 0;
	} else {
		readline();
		return 0;
	}

	// Now expand current line
	pinx = 0;
	while (ch) {
		if (ctype[ch] & CISSPACE) {
			keepch(' ');
			while (ctype[ch] & CISSPACE)
				gch();
		} else if (ch == '"') {
			keepch(gch());
			while (ch != '"') {
				if (ch == '\\') {
					keepch(gch());
					keepch(gch());
				} else if (!ch) {
					error("no quote");
					break;
				} else
					keepch(gch());
			}
			keepch('\"');
			gch();
		} else if (ch == '\'') {
			keepch(gch());
			while (ch != '\'') {
				if (ch == '\\') {
					keepch(gch());
					keepch(gch());
				} else if (!ch) {
					error("no apostrophe");
					break;
				} else
					keepch(gch());
			}
			keepch('\'');
			gch();
		} else if ((ch == '/') && (nch == '*')) {
			bump(2);
			while ((ch != '*') || (nch != '/')) {
				if (ch)
					bump(1);
				else {
					readline();
					if (!inphdl)
						break;
				}
			}
			bump(2);

		} else if ((ch == '/') && (nch == '/')) {
			// double-slash comment. Erase until end-of-line.
			kill();
		} else if (len = dohash(lptr, &sname)) {
			if (mptr = findmac(sname)) {
				cptr = mptr[MEXPAND];
				while (i = *cptr++)
					keepch(i);
				bump(len);
			} else {
				while (len--)
					keepch(gch());
			}
		} else
			keepch(gch());
	}

	// make line available
	keepch(0);
	if (pinx == PBUFMAX)
		error("line too long");
	line = pbuf;
	bump(0);

	// Copy line to listing
	if (maklis) {
		int len;
		len = strlen(line);
		while (len && (ctype[line[len - 1]] & CISSPACE))
			len--;
		fprintf(outhdl, "; %d %s\n", inchdl ? inclnr : inplnr, line);
	}
}

//*
//*
//* Converts and Matches
//*
//*

/* 
 * Skip all spaces in current line
 */
white() {
	while (ctype[ch] & CISSPACE)
		gch();
}

/* 
 * Skip all spaces until next non-space
 */
blanks() {
	while (!ch || ctype[ch] & CISSPACE) {
		if (!ch) {
			if (!inphdl)
				break;
			else
				preprocess();
		} else
			gch();
	}
}

/*
 * Return 'index' if both strings match
 */
streq(register char *str1, register char *str2) {
	register int i;

	i = 0;
	while (str2[i]) {
		if (str1[i] != str2[i])
			return 0;
		i++;
	}
	return i;
}

/*
 * Return 'index' if str2 matches alphanumeric token str1
 */
astreq(register char *str1, register char *str2) {
	register int i;

	i = 0;
	while (str2[i]) {
		if (str1[i] != str2[i])
			return 0;
		i++;
	}
	if (ctype[str1[i]] & CSYMNEXT)
		return 0;
	return i;
}

/*
 * Return 'index' if start next token equals 'lit'
 */
match(char *lit) {
	register int i;

	blanks();
	if (i = streq(lptr, lit)) {
		bump(i);
		return 1;
	}
	return 0;
}

/*
 * Return 'index' if next token equals 'lit'
 */
amatch(char *lit) {
	register int i;

	blanks();
	if (i = astreq(lptr, lit)) {
		bump(i);
		return 1;
	}
	return 0;
}

/*
 * Return 'true' if next operator equals 'lit'
 */
omatch(register char *lit) {
	blanks();
	if (lptr[0] != lit[0])
		return 0;
	if (!lit[1]) {
		if ((lptr[1] == '=') || (lptr[1] == lptr[0]))
			return 0;
		bump(1);
	} else {
		if (lptr[1] != lit[1])
			return 0;
		if (!lit[2]) {
			if (lptr[2] == '=')
				return 0;
			bump(2);
		} else {
			if (lptr[2] != lit[2])
				return 0;
			bump(3);
		}
	}
	return 1;
}

//*
//*
//* Error routines
//*
//*

/*
 * Generate error messages
 */
warning(char *msg) {
	if (inchdl)
		printf("'%s' ", incfn);
// Display original line
	printf("%d: %s\n%%%s\n", inchdl ? inclnr : inplnr, sbuf, msg);
	fprintf(outhdl, ";%% %s\n", msg);
}

error(char *msg) {
	warning(msg);
	errflag = 1;
}

fatal(char *msg) {
	error(msg);
	exit(1);
}

exprerr() {
	error("Invalid expression");
	gencode(TOK_ILLEGAL);
	junk();
}

needlval() {
	error("must be lvalue");
	gencode(TOK_ILLEGAL);
}

illname() {
	error("illegal symbol name");
	junk();
}

multidef() {
	error("identifier already defined");
}

undef() {
	error("identifier undefined");
}

needtoken(char *str) {
	char txt[32], *p1, *p2;

	if (!match(str)) {
		p1 = txt;
		p2 = "Expected ";
		while (*p1++ = *p2++);
		--p1; // Overwrite terminator
		while (*p1++ = *str++);
		error(txt);
	}
}

/*
 * Skip current symbol
 */
junk() {
	if (ctype[ch] & CSYMNEXT) {
		while (ctype[ch] & CSYMNEXT)
			gch();
	} else {
		while (ch & (~ctype[ch] & CSYMNEXT))
			gch();
	}
	blanks();
}

/*
 * Test for end-of-statement or end-of-file
 */
endst() {
	blanks();
	return (!ch || (ch == ';'));
}

/*
 * semicolon enforcer
 */
ns() {
	if (!match(";")) {
		error("no semicolon");
		junk();
	} else
		errflag = 0;
}

//*
//*
//* Main
//*
//*

/*
 * Execution starts here
 */
main(int argc, int *argv) {
	register int i, j;

	initialize(); // initialize all variables

	startup(argv);       // Process commandline options
	openfile();          // Open all files
	preprocess();        // fetch first line
	toseg(CODESEG);      // setup initial segment //
	parse();             // GO !!!
	if (iflevel)
		error("no closing #endif");
	if (!ccode)
		error("no closing #endasm");
	fprintf(outhdl, "\t.END\n");

	j = 0;
	for (i = 0; i < NAMEMAX; i++) if (namech[i]) j++;
	fprintf(outhdl, "; Names        : %5d/%5d\n", j, NAMEMAX);
	for (i = 0; i < SYMMAX && syms[i * ILAST + INAME]; i++);
	fprintf(outhdl, "; Identifiers  : %5d/%5d\n", i, SYMMAX);
	fprintf(outhdl, "; Macros       : %5d/%5d\n", macinx, MACMAX);
	fprintf(outhdl, "; Local labels : %5d\n", nxtlabel);
	for (i = 1; (i < SWMAX) && sw[i * SLAST + SLABEL]; i++);
	fprintf(outhdl, "; Switch cases : %5d/%5d\n", i - 1, SWMAX);

	return errflag;
}

//*
//*
//* General output routines
//*
//*

/*
 * Generate a assembler statement
 */
genopc(int opc) {
	switch (opc) {
	case TOK_ILLEGAL: fprintf(outhdl, "\tILLEGAL\t"); break;
	case TOK_ADD : fprintf(outhdl, "\tADD\t"); break;
	case TOK_SUB : fprintf(outhdl, "\tSUB\t"); break;
	case TOK_MUL : fprintf(outhdl, "\tMUL\t"); break;
	case TOK_DIV : fprintf(outhdl, "\tDIV\t"); break;
	case TOK_MOD : fprintf(outhdl, "\tMOD\t"); break;
	case TOK_OR  : fprintf(outhdl, "\tOR\t"); break;
	case TOK_XOR : fprintf(outhdl, "\tXOR\t"); break;
	case TOK_AND : fprintf(outhdl, "\tAND\t"); break;
	case TOK_LSR : fprintf(outhdl, "\tLSR\t"); break;
	case TOK_LSL : fprintf(outhdl, "\tLSL\t"); break;
	case TOK_NEG : fprintf(outhdl, "\tNEG\t"); break;
	case TOK_NOT : fprintf(outhdl, "\tNOT\t"); break;
	case TOK_BEQ  : fprintf(outhdl, "\tBEQ\t"); break;
	case TOK_BNE  : fprintf(outhdl, "\tBNE\t"); break;
	case TOK_BLT  : fprintf(outhdl, "\tBLT\t"); break;
	case TOK_BLE  : fprintf(outhdl, "\tBLE\t"); break;
	case TOK_BGT  : fprintf(outhdl, "\tBGT\t"); break;
	case TOK_BGE  : fprintf(outhdl, "\tBGE\t"); break;
	case TOK_LDB: fprintf(outhdl, "\tLDB\t"); break;
	case TOK_LDW: fprintf(outhdl, "\tLDW\t"); break;
	case TOK_LDR: fprintf(outhdl, "\tLDR\t"); break;
	case TOK_LEA : fprintf(outhdl, "\tLDA\t"); break;
	case TOK_CMP : fprintf(outhdl, "\tCMP\t"); break;
	case TOK_TST : fprintf(outhdl, "\tTST\t"); break;
	case TOK_STB: fprintf(outhdl, "\tSTB\t"); break;
	case TOK_STW: fprintf(outhdl, "\tSTW\t"); break;
	case TOK_JMP : fprintf(outhdl, "\tJMP\t"); break;
	case TOK_JSB : fprintf(outhdl, "\tJSB\t"); break;
	case TOK_RSB : fprintf(outhdl, "\tRSB\t"); break;
	case TOK_PSHB: fprintf(outhdl, "\tPSHB\t"); break;
	case TOK_PSHW: fprintf(outhdl, "\tPSHW\t"); break;
	case TOK_PSHA: fprintf(outhdl, "\tPSHA\t"); break;
	case TOK_PSHR: fprintf(outhdl, "\tPSHR\t"); break;
	case TOK_POPR: fprintf(outhdl, "\tPOPR\t"); break;
	default:
		fprintf(outhdl, "\tTOK_%d\t", opc);
		break;
	}
}

gencode(int opc) {
	genopc(opc);

	fprintf(outhdl, "\n");
}

gencode_L(int opc, int lbl) {
	genopc(opc);

	fprintf(outhdl, "_%d\n", lbl);
}

gencode_R(int opc, int reg1, int reg2) {
	genopc(opc);

	if (reg1)
		fprintf(outhdl, "R%d,", reg1);
	fprintf(outhdl, "R%d\n", reg2);
}

gencode_I(int opc, int reg, int imm) {
	// sign extend
	imm |= -(imm & (1 << SBIT));

	genopc(opc);

	if (reg)
		fprintf(outhdl, "R%d,", reg);
	fprintf(outhdl, "%d\n", imm);
}

gencode_ADJSP(int imm) {
	// sign extend
	imm |= -(imm & (1 << SBIT));

	if (imm == BPW)
		fprintf(outhdl, "\tADD\tR%d,R%d\n", REG_SP, REG_BPW);
	else if (imm == -BPW)
		fprintf(outhdl, "\tSUB\tR%d,R%d\n", REG_SP, REG_BPW);
	else {
		int reg;
		reg = allocreg();
		fprintf(outhdl, "\tLDA\tR%d,%d\n", reg, imm);
		fprintf(outhdl, "\tADD\tR%d,R%d\n", REG_SP, reg);
		freereg(reg);
	}
}

gencode_IND(int opc, int reg, int ofs, int ind) {
	// sign extend
	ofs |= -(ofs & (1 << SBIT));

	genopc(opc);

	if (reg)
		fprintf(outhdl, "R%d,", reg);
	if (ofs)
		fprintf(outhdl, "%d", ofs);
	fprintf(outhdl, "(R%d)\n", ind);
}

gencode_M(int opc, int reg, register int lval[]) {
	genopc(opc);

	if (reg)
		fprintf(outhdl, "R%d,", reg);

	// apply any stack ajustments
	if ((lval[LREG1] == REG_SP) || (lval[LREG2] == REG_SP))
		lval[LVALUE] = lval[LVALUE] - csp;

	if (lval[LNAME]) {
		if (lval[LTYPE] == LABEL)
			fprintf(outhdl, "_%d", lval[LNAME]);
		else {
			fprintf(outhdl, "_");
			symname(lval[LNAME]);
		}
	}

	// sign extend
	int ofs;
	ofs = lval[LVALUE] | -(lval[LVALUE] & (1 << SBIT));

	if (lval[LVALUE] > 0)
		fprintf(outhdl, "+%d", ofs);
	else if (lval[LVALUE] < 0)
		fprintf(outhdl, "%d", ofs);
	if (lval[LREG1]) {
		fprintf(outhdl, "(R%d", lval[LREG1]);
		if (lval[LREG2])
			fprintf(outhdl, ",R%d", lval[LREG2]);
		fprintf(outhdl, ")");
	}

	fprintf(outhdl, "\n");
}

/*
 * change to a new segment
 * may be called with NULL, CODESEG, or DATASEG
 */
toseg(register int newseg) {
	prevseg = currseg;
	if (currseg == newseg)
		return 0;
	if (newseg == CODESEG)
		fprintf(outhdl, "\t.CODE\n");
	else if (newseg == DATASEG)
		fprintf(outhdl, "\t.DATA\n");
	else if (newseg == UDEFSEG)
		fprintf(outhdl, "\t.UDEF\n");
	currseg = newseg;
}

