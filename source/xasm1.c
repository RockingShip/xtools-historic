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
//* X-Assembler.  Part 1, I/O Support
//*

#define EXTERN
#include "xasm.h"

//*
//*
//* Compiler startup routines
//*
//*

/*
 * Initialize all variables
 */
add_res(char *opc, int typ, int val) {
	char buf[20];
	int hash;
	register int *p;
	register char *cp1, *cp2;

	// upper case
	dohash(opc, &hash);
	p = &names[hash * NLAST];
	p[NTYPE] = typ;
	p[NVALUE] = val;

	// AND lowercase
	cp1 = opc;
	cp2 = buf;
	while (*cp1)
		*cp2++ = ((*cp1 >= 'A') && (*cp1 <= 'Z')) ? *cp1++ - 'A' + 'a' : *cp1++;
	*cp2++ = 0;
	dohash(buf, &hash);
	p = &names[hash * NLAST];
	p[NTYPE] = typ;
	p[NVALUE] = val;
}

/*
 * Initialize all variables
 */
initialize() {
	register int i, *p;

	verbose = debug = 0;
	outhdl = lishdl = inphdl = inchdl = 0;
	iflevel = skiplevel = 0;
	macinx = macqinx = 0;
	inclnr = inplnr = 0;
	inpfn[0] = 0;
	datlen = 0;

	// character properties
	for (i = '0'; i <= '9'; ++i)
		ctype[i] = CISDIGIT | CISXDIGIT | CSYMNEXT;
	for (i = 'A'; i <= 'F'; ++i)
		ctype[i] = CISUPPER | CISXDIGIT | CSYMFIRST | CSYMNEXT;
	for (i = 'G'; i <= 'Z'; ++i)
		ctype[i] = CISUPPER | CSYMFIRST | CSYMNEXT;
	for (i = 'a'; i <= 'f'; ++i)
		ctype[i] = CISLOWER | CISXDIGIT | CSYMFIRST | CSYMNEXT;
	for (i = 'g'; i <= 'z'; ++i)
		ctype[i] = CISLOWER | CSYMFIRST | CSYMNEXT;
	ctype['_'] = CSYMFIRST | CSYMNEXT;
	ctype['.'] = CSYMFIRST | CSYMNEXT;
	ctype[' '] = CISSPACE;
	ctype['\f'] = CISSPACE;
	ctype['\n'] = CISSPACE;
	ctype['\r'] = CISSPACE;
	ctype['\t'] = CISSPACE;
	ctype['\v'] = CISSPACE;

	// reset table
	for (i = 0; i < NAMEMAX; ++i) {
		p = &names[i * NLAST];
		p[NCHAR] = p[NTYPE] = 0;
	}

	// reserve first entry so it terminates lists
	names[0 * NLAST + NCHAR] = '?';

	// reset positions
	pass = 1;
	curseg = CODESEG;
	curpos[CODESEG] = curpos[DATASEG] = curpos[TEXTSEG] = curpos[UDEFSEG] = 0;
	maxpos[CODESEG] = maxpos[DATASEG] = maxpos[TEXTSEG] = maxpos[UDEFSEG] = 0;

	// reserved words
	add_res("ILLEGAL", OPCODE, OPC_ILLEGAL);
	add_res("ADD", OPCODE, OPC_ADD);
	add_res("SUB", OPCODE, OPC_SUB);
	add_res("MUL", OPCODE, OPC_MUL);
	add_res("DIV", OPCODE, OPC_DIV);
	add_res("MOD", OPCODE, OPC_MOD);
	add_res("OR", OPCODE, OPC_OR);
	add_res("XOR", OPCODE, OPC_XOR);
	add_res("AND", OPCODE, OPC_AND);
	add_res("LSR", OPCODE, OPC_LSR);
	add_res("LSL", OPCODE, OPC_LSL);
	add_res("NEG", OPCODE, OPC_NEG);
	add_res("NOT", OPCODE, OPC_NOT);
	add_res("BEQ", OPCODE, OPC_BEQ);
	add_res("BNE", OPCODE, OPC_BNE);
	add_res("BLT", OPCODE, OPC_BLT);
	add_res("BLE", OPCODE, OPC_BLE);
	add_res("BGT", OPCODE, OPC_BGT);
	add_res("BGE", OPCODE, OPC_BGE);
	add_res("LDB", OPCODE, OPC_LDB);
	add_res("LDW", OPCODE, OPC_LDW);
	add_res("LDR", OPCODE, OPC_LDR);
	add_res("LDA", OPCODE, OPC_LEA);
	add_res("CMP", OPCODE, OPC_CMP);
	add_res("STB", OPCODE, OPC_STB);
	add_res("STW", OPCODE, OPC_STW);
	add_res("JMP", OPCODE, OPC_JMP);
	add_res("JSB", OPCODE, OPC_JSB);
	add_res("RSB", OPCODE, OPC_RSB);
	add_res("PSHR", OPCODE, OPC_PSHR);
	add_res("POPR", OPCODE, OPC_POPR);
	add_res("PSHB", OPCODE, OPC_PSHB);
	add_res("PSHW", OPCODE, OPC_PSHW);
	add_res("PSHA", OPCODE, OPC_PSHA);
	add_res("SVC", OPCODE, OPC_SVC);
	add_res(".CODE", PSEUDO, PSEUDO_CODE);
	add_res(".DATA", PSEUDO, PSEUDO_DATA);
	add_res(".TEXT", PSEUDO, PSEUDO_TEXT);
	add_res(".UDEF", PSEUDO, PSEUDO_UDEF);
	add_res(".ORG", PSEUDO, PSEUDO_ORG);
	add_res(".END", PSEUDO, PSEUDO_END);
	add_res(".DCB", PSEUDO, PSEUDO_DCB);
	add_res(".DCW", PSEUDO, PSEUDO_DCW);
	add_res(".DSB", PSEUDO, PSEUDO_DSB);
	add_res(".DSW", PSEUDO, PSEUDO_DSW);
	add_res("R0", REGISTER, 0);
	add_res("R1", REGISTER, 1);
	add_res("R2", REGISTER, 2);
	add_res("R3", REGISTER, 3);
	add_res("R4", REGISTER, 4);
	add_res("R5", REGISTER, 5);
	add_res("R6", REGISTER, 6);
	add_res("R7", REGISTER, 7);
	add_res("R8", REGISTER, 8);
	add_res("R9", REGISTER, 9);
	add_res("R10", REGISTER, 10);
	add_res("R11", REGISTER, 11);
	add_res("R12", REGISTER, 12);
	add_res("R13", REGISTER, 13);
	add_res("R14", REGISTER, 14);
	add_res("R15", REGISTER, 15);
	add_res(".", POINT, 0);

	hier_str[ 0] = "|";  hier_oper[ 0] = REL_OR;    // expr_or
	hier_str[ 1] = 0;
	hier_str[ 2] = "^";  hier_oper[ 2] = REL_XOR;   // expr_xor
	hier_str[ 3] = 0;
	hier_str[ 4] = "&";  hier_oper[ 4] = REL_AND;   // expr_and
	hier_str[ 5] = 0;
	hier_str[ 6] = ">>"; hier_oper[ 6] = REL_LSR;   // expr_shift
	hier_str[ 7] = "<<"; hier_oper[ 7] = REL_LSL;
	hier_str[ 8] = 0;
	hier_str[ 9] = "+";  hier_oper[ 9] = REL_ADD;   // expr_addsub
	hier_str[10] = "-";  hier_oper[10] = REL_SUB;
	hier_str[11] = 0;
	hier_str[12] = "*";  hier_oper[12] = REL_MUL;   // expr_muldiv
	hier_str[13] = "/";  hier_oper[13] = REL_DIV;
	hier_str[14] = "%";  hier_oper[14] = REL_MOD;
	hier_str[15] = 0;
}

/*
 * Process commandline
 */
usage() {
	printf("X-Assembler, Version %s\n\n", getversion());

	printf("usage: xasm <file>[.<ext>]\n");
	printf("  -a <file>[.<ext>]]\tListing\n");
	printf("  -c <file>[.<ext>]]\tObject output\n");
	printf("  -v\t\t\tVerbose\n");
	exit(1);
}

fext(char *out, char *path, char *ext, int force) {
	char *p;
	int baselen;

	baselen = 0;
	for (p = path; *p; ++p) {
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

startup(register int *argv) {
	++argv; // skip argv[0]
	while (*argv) {
		register char *arg;
		arg = *argv++;

		if (*arg != '-') {
			fext(inpfn, arg, ".xs", 0);
			if (!outfn[0])
				fext(outfn, arg, ".xo", 1);
		} else {
			// Process option
			++arg;
			switch (*arg++) {
			case 'a':
				if (!*arg && *argv)
					arg = *argv++;
				if (!*arg || *arg == '-')
					fext(lisfn, inpfn, ".lis", 1);
				else
					fext(lisfn, arg, ".lis", 0);
				break;
			case 'c':
				if (!*arg && *argv)
					arg = *argv++;
				if (!*arg || *arg == '-')
					usage();
				else
					fext(outfn, arg, ".xo", 0);
				break;
			case 'd':
				debug = 1;
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
 * Bump next characters in line (n = #chars or 0 for initialization)
 */
bump(register int n) {
	lptr = n ? lptr + n : line;
	nch = (ch = lptr[0]) ? lptr[1] : 0;
}

//*
//*
//* Symboltable routines
//*
//*

foutname(register int hash) {
	register int i;

	i = names[hash * NLAST + NTAB];
	if (i)
		foutname(i);
	fprintf(outhdl, "%c", names[hash * NLAST + NCHAR]);
}

outname(register int hash) {
	register int i;

	i = names[hash * NLAST + NTAB];
	if (i)
		outname(i);
	printf("%c", names[hash * NLAST + NCHAR]);
}

lenname(register int hash) {
	register int i;

	i = names[hash * NLAST + NTAB];
	if (i)
		return lenname(i) + 1;
	return 1;
}

/*
 * Get the (unique) hashed value for symbol, return length
 */
dohash(register char *ident, int *retval) {
	register int start, hash, tab, len, *p;

	if (!(ctype[*ident] & CSYMFIRST))
		return 0; // Not a symbol

	tab = 0;
	len = 0;
	hash = 0;
	while (ctype[*ident] & CSYMNEXT) {
		start = hash = (hash + *ident * *ident) % NAMEMAX;
		while (1) {
			p = &names[hash * NLAST];
			if ((p[NCHAR] == *ident) && (p[NTAB] == tab)) {
				tab = hash;
				break; // Inner loop
			} else if (!p[NCHAR]) {
				p[NCHAR] = *ident;
				p[NTAB] = tab;
				tab = hash;
				break; // Inner loop
			} else {
				hash += *ident;
				if (hash >= NAMEMAX)
					hash -= NAMEMAX;
				if (hash == start)
					fatal("name table overflow");
			}
		}
		++ident;
		++len;
	}
	*retval = tab;
	return len;
}

//*
//*
//* Preprocessor
//*
//*

/*
 * open an include file
 */
doinclude() {
	register char *p;

	blanks();
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
	ch = 0;
}

/*
 * Declare/define a macro
 */
declmac() {
	int sname;
	register int len;
	register int *mptr;

	blanks();
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
		blanks();
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

keepch(int c) {
	if (pinx < PBUFMAX)
		pbuf[pinx++] = c;
}

readline() {
	sbuf[0] = 0;
	if (inphdl)
		while (!sbuf[0]) {
			if (inchdl) {
				if (!fgets(sbuf, SBUFMAX, inchdl)) {
					fclose(inchdl);
					inchdl = 0;
					continue;
				}
				++inclnr;
			} else if (inphdl) {
				if (!fgets(sbuf, SBUFMAX, inphdl)) {
					fclose(inphdl);
					inphdl = 0;
					break;
				}
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
		blanks();
		if (!ch)
			continue; // Try again

		if (amatch("#ifdef")) {
			++iflevel;
			if (!skiplevel) {
				blanks();
				if (!dohash(lptr, &sname))
					error("identifier expected");
				else if (!findmac(sname))
					skiplevel = iflevel;
			}
		} else if (amatch("#ifndef")) {
			++iflevel;
			if (!skiplevel) {
				blanks();
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

	// get inputline
	ifline();
	if (!inphdl)
		return;

	// Now expand current line
	pinx = 0;
	while (ch) {
		if (ch <= ' ') {
			keepch(' ');
			while (ch && (ch <= ' '))
				gch();
		} else if (ch == '"') {
			keepch(gch());
			while (ch != '"') {
				if (ch == '\\') {
					keepch(gch());
					keepch(gch());
				} else if (!ch) {
					if (pass == 1)
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
					if (pass == 1)
						error("no apostrophe");
					break;
				} else
					keepch(gch());
			}
			keepch('\'');
			gch();
		} else if (ch == ';') {
			break;
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
	if (lishdl) {
		if (curseg == CODESEG)
			fprintf(lishdl, ";C%04x: %s\n", curpos[CODESEG] & 0xffff, pbuf);
		else if (curseg == DATASEG)
			fprintf(lishdl, ";D%04x: %s\n", curpos[DATASEG] & 0xffff, pbuf);
		else if (curseg == TEXTSEG)
			fprintf(lishdl, ";T%04x: %s\n", curpos[TEXTSEG] & 0xffff, pbuf);
		else
			fprintf(lishdl, ";U%04x: %s\n", curpos[UDEFSEG] & 0xffff, pbuf);
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
blanks() {
	while (1) {
		if (ch == ';')
			*lptr = ch = 0;

		if (!ch || !(ctype[ch] & CISSPACE))
			break;

		gch();
	}
}

/*
 * Return 'index' if start next token equals 'lit'
 */
match(register char *lit) {
	register int i;

	blanks();

	i = 0;
	while (lit[i]) {
		if (lptr[i] != lit[i])
			return 0;
		++i;
	}

	bump(i);
	return 1;
}

/*
 * Return 'index' if next token equals 'lit'
 */
amatch(register char *lit) {
	register int i;

	blanks();

	i = 0;
	while (lit[i]) {
		if (lptr[i] != lit[i])
			return 0;
		++i;
	}
	if (ctype[lptr[i]] & CSYMNEXT)
		return 0;

	bump(i);
	return 1;
}

/*
 * Return 'true' if next operator equals 'lit'
 */
omatch(register char *lit) {
	blanks();
	if (lptr[0] != lit[0])
		return 0;
	if (lit[1]) {
		if (lptr[1] != lit[1])
			return 0;
		bump(2);
	} else {
		if ((lptr[1] == '=') || (lptr[1] == lptr[0]))
			return 0;
		bump(1);
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
error(char *msg) {
	if (inchdl)
		printf("'%s' ", incfn);
	// Display original line
	printf("%d: %s\n%%%s\n", inchdl ? inclnr : inplnr, sbuf, msg);
	if (lishdl)
		fprintf(lishdl, ";%d %%%s\n", inchdl ? inclnr : inplnr, msg);

	errflag = 1;
}

fatal(char *msg) {
	error(msg);
	exit(1);
}

exprerr() {
	error("Invalid expression");
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
	register int i, j, *p;

	initialize();        // initialize all variables
	startup(argv);       // Process commandline options

	{
		inphdl = mustopen(inpfn, "r");

		macinx = macqinx = 0;
		iflevel = skiplevel = 0;
		inclnr = inplnr = 0;
		curseg = CODESEG;
		curpos[CODESEG] = curpos[DATASEG] = curpos[TEXTSEG] = curpos[UDEFSEG] = 0;
		datlen = 0;

		pass = 1;
		if (verbose)
			printf("Pass 1\n");

		preprocess(); // fetch first line
		parse(); // GO !!!
		save_seg_size();
		if (iflevel)
			error("no closing #endif");

		if (!inphdl)
			printf("%%missing .END statement\n");
		else
			fclose(inphdl);
	}

	{
		inphdl = mustopen(inpfn, "r");
		outhdl = mustopen(outfn, "w");
		if (lisfn[0])
			lishdl = mustopen(lisfn, "w");

		macinx = macqinx = 0;
		iflevel = skiplevel = 0;
		inclnr = inplnr = 0;
		curseg = CODESEG;
		curpos[CODESEG] = curpos[DATASEG] = curpos[TEXTSEG] = curpos[UDEFSEG] = 0;
		datlen = 0;

		pass = 2;
		if (verbose)
			printf("Pass 2\n");

		preprocess(); // fetch first line
		parse(); // GO !!!
		if (iflevel)
			error("no closing #endif");

		sto_cmd(REL_END, 0);
	}

	if (lishdl) {
		fprintf(lishdl, "CODE   : 0x%04x (%5d)\n", maxpos[CODESEG] & 0xffff, maxpos[CODESEG] & 0xffff);
		fprintf(lishdl, "DATA   : 0x%04x (%5d)\n", maxpos[DATASEG] & 0xffff, maxpos[DATASEG] & 0xffff);
		fprintf(lishdl, "TEXT   : 0x%04x (%5d)\n", maxpos[TEXTSEG] & 0xffff, maxpos[TEXTSEG] & 0xffff);
		fprintf(lishdl, "UDEF   : 0x%04x (%5d)\n", maxpos[UDEFSEG] & 0xffff, maxpos[UDEFSEG] & 0xffff);
		fprintf(lishdl, "Macros : %5d(%5d)\n", macinx, MACMAX);
		j = 0;
		for (i = 0; i < NAMEMAX; ++i) if (names[i * NLAST + NCHAR]) ++j;
		fprintf(lishdl, "Names  : %5d(%5d)\n", j, NAMEMAX);
	}

	return errflag;
}

//*
//*
//* General output routines
//*
//*

write_byte(char byte) {
	fwrite(&byte, 1, 1, outhdl);
}

write_word(int word) {
	char arr[2];
	arr[0] = word >> 8;
	arr[1] = word;

	fwrite(arr, 1, 2, outhdl);
}

sto_flush() {
	char cval;
	register int i, j;

	if ((pass == 2) && datlen) {
		if (!debug) {
			cval = -datlen;
			write_byte(cval);
			fwrite(datbuf, 1, datlen, outhdl);
		} else {
			i = 0;
			while (i < datlen) {
				for (j = 0; j < 16; ++j) {
					fprintf(outhdl, "0x%02x", (i >= datlen) ? 0 : datbuf[i]);
					if (++i >= datlen)
						break;
					if (j != 15)
						fputc(',', outhdl);
				}
				fputc('\n', outhdl);
			}
		}

		datlen = 0;
	}
}

sto_data(int val, int size) {
	if (size == BPW) {
		// store upper byte
		if (datlen == 128)
			sto_flush();
		datbuf[datlen++] = val >> 8;
	}
	// store lower byte
	if (datlen == 128)
		sto_flush();
	datbuf[datlen++] = val;
}

sto_cmd(int cmd, int val) {
	char cval;
	register int *p;

	sto_flush();
	cval = val;
	if (pass == 2) {
		if (!debug)
			switch (cmd) {
			case REL_END:
			case REL_ADD:
			case REL_SUB:
			case REL_MUL:
			case REL_DIV:
			case REL_MOD:
			case REL_LSR:
			case REL_LSL:
			case REL_AND:
			case REL_OR:
			case REL_XOR:
			case REL_SWAP:
			case REL_POPB:
			case REL_POPW:
				write_byte(cmd);
				break;
			case REL_PUSHB:
			case REL_PUSHW:
				if ((val >= -128) && (val <= 127))
					cmd = REL_PUSHB;
				if (cmd == REL_PUSHB) {
					cval = val;
					write_byte(cmd);
					write_byte(cval);
				} else {
					write_byte(cmd);
					write_word(val);
				}
				break;
			case REL_CODEB:
			case REL_CODEW:
				if ((val >= -128) && (val <= 127))
					cmd = REL_CODEB;
				if (cmd == REL_CODEB) {
					write_byte(cmd);
					write_byte(cval);
				} else {
					write_byte(cmd);
					write_word(val);
				}
				break;
			case REL_DATAB:
			case REL_DATAW:
				if ((val >= -128) && (val <= 127))
					cmd = REL_DATAB;
				if (cmd == REL_DATAB) {
					write_byte(cmd);
					write_byte(cval);
				} else {
					write_byte(cmd);
					write_word(val);
				}
				break;
			case REL_TEXTB:
			case REL_TEXTW:
				if ((val >= -128) && (val <= 127))
					cmd = REL_TEXTB;
				if (cmd == REL_TEXTB) {
					write_byte(cmd);
					write_byte(cval);
				} else {
					write_byte(cmd);
					write_word(val);
				}
				break;
			case REL_UDEFB:
			case REL_UDEFW:
				if ((val >= -128) && (val <= 127))
					cmd = REL_UDEFB;
				if (cmd == REL_UDEFB) {
					write_byte(cmd);
					write_byte(cval);
				} else {
					write_byte(cmd);
					write_word(val);
				}
				break;
			case REL_SYMBOL:
				write_byte(cmd);
				write_byte(lenname(val));
				foutname(val);
				break;
			case REL_CODEDEF:
			case REL_DATADEF:
			case REL_TEXTDEF:
			case REL_UDEFDEF:
				p = &names[val * NLAST];
				write_byte(cmd);
				write_word(p[NVALUE]);
				cval = lenname(val);
				write_byte(cval);
				foutname(val);
				break;
			case REL_CODEORG:
			case REL_DATAORG:
			case REL_TEXTORG:
			case REL_UDEFORG:
			case REL_DSB:
				write_byte(cmd);
				write_word(val);
				break;
			default:
				printf("unimplemented OBJECT cmd: %d\n", cmd);
				exit(1);
				break;
			}
		else {
			switch (cmd) {
			case REL_END: fputs("END\n", outhdl); break;
			case REL_ADD: fputs("ADD\n", outhdl); break;
			case REL_SUB: fputs("SUB\n", outhdl); break;
			case REL_MUL: fputs("MUL\n", outhdl); break;
			case REL_DIV: fputs("DIV\n", outhdl); break;
			case REL_MOD: fputs("MOD\n", outhdl); break;
			case REL_LSR: fputs("LSR\n", outhdl); break;
			case REL_LSL: fputs("LSL\n", outhdl); break;
			case REL_AND: fputs("AND\n", outhdl); break;
			case REL_OR: fputs("OR\n", outhdl); break;
			case REL_XOR: fputs("XOR\n", outhdl); break;
			case REL_SWAP: fputs("SWAP\n", outhdl); break;
			case REL_POPB: fputs("POPB\n", outhdl); break;
			case REL_POPW: fputs("POPW\n", outhdl); break;
			case REL_PUSHB: fputs("PUSHB %d\n", val, outhdl); break;
			case REL_PUSHW: fputs("PUSHW %d\n", val, outhdl); break;
			case REL_CODEB: fputs("CODEB %d\n", val, outhdl); break;
			case REL_CODEW: fputs("CODEW %d\n", val, outhdl); break;
			case REL_DATAB: fputs("DATAB %d\n", val, outhdl); break;
			case REL_DATAW: fputs("DATAW %d\n", val, outhdl); break;
			case REL_TEXTB: fputs("TEXTB %d\n", val, outhdl); break;
			case REL_TEXTW: fputs("TEXTW %d\n", val, outhdl); break;
			case REL_UDEFB: fputs("UDEFB %d\n", val, outhdl); break;
			case REL_UDEFW: fputs("UDEFW %d\n", val, outhdl); break;
			case REL_DSB: fputs("DSB %d\n", val, outhdl); break;
			case REL_CODEORG: fputs("CODEORG %d\n", val, outhdl); break;
			case REL_DATAORG: fputs("DATAORG %d\n", val, outhdl); break;
			case REL_TEXTORG: fputs("TEXTORG %d\n", val, outhdl); break;
			case REL_UDEFORG: fputs("UDEFORG %d\n", val, outhdl); break;
			case REL_SYMBOL:
				fputs("SYMBOL ", outhdl);
				foutname(val);
				fputc('\n', outhdl);
				break;
			case REL_CODEDEF:
				p = &names[val * NLAST];
				fprintf(outhdl, "CODEDEF %d,", p[NVALUE]);
				foutname(val);
				fputc('\n', outhdl);
				break;
			case REL_DATADEF:
				p = &names[val * NLAST];
				fprintf(outhdl, "DATADEF %d,", p[NVALUE]);
				foutname(val);
				fputc('\n', outhdl);
				break;
			case REL_TEXTDEF:
				p = &names[val * NLAST];
				fprintf(outhdl, "TEXTDEF %d,", p[NVALUE]);
				foutname(val);
				fputc('\n', outhdl);
				break;
			case REL_UDEFDEF:
				p = &names[val * NLAST];
				fprintf(outhdl, "UDEFDEF %d,", p[NVALUE]);
				foutname(val);
				fputc('\n', outhdl);
				break;
			default:
				fprintf(outhdl, "cmd: %d\n", cmd);
				break;
			}
		}
	}
}
