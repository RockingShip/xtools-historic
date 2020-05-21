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
//* xcc.h -- Symbol definitions for X-C-Compiler
//*

/* 
 * Compiler dependent parameters
 */

enum {
	LITMAX = 1500,		// literal pool
	MACMAX = 300,		// Number of definable macro's
	MACQMAX = MACMAX * 7,	// Expansiontable for macro's
	NAMEMAX = 997,		// Size of nametable !!! MUST BE PRIME !!!
	PATHMAX = 80,		// Length of filename
	PBUFMAX = 512,		// Size of preprocessor buffer
	SBUFMAX = 256,		// Size of source buffer
	SWMAX = 100,		// Number of switch cases
	SYMMAX = 300,		// Number of identifiers
};

/*
 * Machine dependent parmeters
 */

enum {
	BPW = 2,		// Bytes per word
	SBIT = 15,		// Bit number of signed bit
};

/*
 * Possible values for "TYPE"
 */

enum {
	CONSTANT = 1,
	VARIABLE,
	ARRAY,
	LABEL,
	FUNCTION,
	EXPR,
	BRANCH,
};

/*
 * Possible values for "CLASS"
 */

enum {
	STATIC = 1,	// static global variables
	SP_AUTO,	// local variables
	AP_AUTO,	// procedure parameters
	EXTERNAL,	// external global variables
	AUTOEXT,	// referenced names
	GLOBAL,		// global variables
	REGISTER,	// register variables
};

/*
 * Possible values for "EA"
 */

enum {
	EA_ADDR = 1,
	EA_IND,
	EA_REG,
};

/*
 * Definitions for lval  (Code generated node)
 */

enum {
	LTYPE = 0,
	LPTR,
	LSIZE,
	LEA,
	LNAME,
	LVALUE,
	LREG1,
	LREG2,
	LFALSE,
	LTRUE,
	LLAST,
};

/*
 * Definitions for identifiers
 */

enum {
	INAME = 0,
	ITYPE,
	IPTR,
	ICLASS,
	IVALUE,
	ISIZE,
	ILAST,
};

/*
 * Definitions for macro's
 */

enum {
	MNAME = 0,
	MEXPAND,
	MLAST,
};

/*
 * Definitions for switches
 */

enum {
	SCASE = 0,
	SLABEL,
	SLAST,
};

/*
 * Reserved registers
 */

enum {
	REG_SP = 15,
	REG_AP = 14,
	REG_BPW = 13,
	REG_4 = 12,
	REG_1 = 11,
	REG_0 = 10,
	REG_RESVD = ((1 << REG_SP) | (1 << REG_AP) | (1 << REG_BPW) | (1 << REG_4) | (1 << REG_1) | (1 << REG_0) | (1 << 1) | (1 << 0)),
};

/*
 * segment names
 */

enum {
	CODESEG = 1,
	DATASEG = 2,
	UDEFSEG = 3,
};

/*
 * Compiler-generated p-codes
 */

enum {
	TOK_ILLEGAL = 1,
	TOK_ADD = 2,
	TOK_SUB = 3,
	TOK_MUL = 4,
	TOK_DIV = 5,
	TOK_MOD = 6,
	TOK_OR = 7,
	TOK_XOR = 8,
	TOK_AND = 9,
	TOK_LSR = 10,
	TOK_LSL = 11,
	TOK_NEG = 12,
	TOK_NOT = 13,
	TOK_BEQ = 14,
	TOK_BNE = 15,
	TOK_BLT = 16,
	TOK_BLE = 17,
	TOK_BGT = 18,
	TOK_BGE = 19,
	TOK_LDB = 20,
	TOK_LDW = 21,
	TOK_LDR = 22,
	TOK_LEA = 23,
	TOK_CMP = 24,
	TOK_TST = 25,
	TOK_STB = 26,
	TOK_STW = 27,
	TOK_JMP = 28,
	TOK_JSB = 29,
	TOK_RSB = 30,
	TOK_PSHR = 31,
	TOK_POPR = 32,
	TOK_PSHB = 33,
	TOK_PSHW = 34,
	TOK_PSHA = 35,
};

/*
 * vharacter properties
 */

enum {
	CISSPACE = 1 << 0,	// is a space
	CISDIGIT = 1 << 1,	// is a digit
	CISXDIGIT = 1 << 2,	// is a hex digit
	CISUPPER = 1 << 3,	// is lowercase
	CISLOWER = 1 << 4,	// is uppercase
	CSYMFIRST = 1 << 5,	// first character of an identifier name
	CSYMNEXT = 1 << 6,	// next character of an identifier name
};

/*
 * Storage
 */

EXTERN int
	argcid,			// hash value of reserved word
	argvid,			// hash value of reserved word
	ccode,			// True for C source, else ASM source
	csp,			// stackpointer seen from scope coding
	currseg,		// Current output segment
	errflag,		// True if an error has occurred
	hier_oper[30],		// Internal translation of the above
	hier_str[30],		// Array containing hierarchical operators
	iflevel,		// #if nesting level
	inchdl,			// handle for .H file
	inclnr,			// Linenumber of .H file
	inphdl,			// handle for .C file
	inplnr,			// Linenumber of .C file
	lishdl,			// handle for .LIS file
	litinx,			// Index to next entry
	litq[LITMAX],
	mac[MACMAX*MLAST],
	macinx,			// Next free entry in mac
	macqinx,		// Next free entry in macq
	maklis,			// Listing -h specified
	nametab[NAMEMAX],	// Name table
	nxtlabel,		// Next label number
	outhdl,			// handle for .ASM file
	pinx,			// Position in preprocessor buffer
	prevseg,		// Previous output segment
	reglock,		// Register locked by 'register' vars
	regsum,			// Summary of all used registers
	reguse,			// Currently used registers
	skiplevel,		// level at which #if skipping starts
	sw[SWMAX*SLAST],
	swinx,			// Position in switch table
	symidx,			// Next free identifier
	syms[SYMMAX*ILAST],	// Symbols/identifiers
	verbose;		// Verbose -v specified

EXTERN char
	ch,			// Current character in line being scanned
	ctype[256],		// character properties
	incfn[PATHMAX],		// include filename
	inpfn[PATHMAX],		// input filename
	*line,			// Pointer to current input buffer
	*lptr,			// Pointer to current character in input buffer
	macq[MACQMAX],
	namech[NAMEMAX],
	nch,			// Next character in line being scanned
	outfn[PATHMAX],		// output filename
	pbuf[PBUFMAX],
	sbuf[SBUFMAX];
