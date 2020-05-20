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

// #define DYNAMIC		// allocate memory dynamically

enum {
	IDMAX = 300,		// Number of identifiers
	LITMAX = 1500,		// literal pool
	MACMAX = 300,		// Number of definable macro's
	MACQMAX = MACMAX * 7,	// Expansiontable for macro's
	NAMEMAX = 997,		// Size of nametable !!! MUST BE PRIME !!!
	PATHMAX = 80,		// Length of filename
	PBUFMAX = 512,		// Size of preprocessor buffer
	SBUFMAX = 256,		// Size of source buffer
	SWMAX = 100,		// Number of switch cases
};

/*
 * Machine dependent parmeters
 */

enum {
	BPW = 2,		// Bytes per word
	SBIT = 15,		// Bit number of signed bit
};

/*
 * Reserved characters
 */

enum {
	BS = 8,
	HT = 9,
	NEWLINE = 10,
	FF = 12,
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
	_ILLEGAL = 1,
	_ADD = 2,
	_SUB = 3,
	_MUL = 4,
	_DIV = 5,
	_MOD = 6,
	_BOR = 7,
	_XOR = 8,
	_BAND = 9,
	_LSR = 10,
	_LSL = 11,
	_NEG = 12,
	_NOT = 13,
	_EQ = 14,
	_NE = 15,
	_LT = 16,
	_LE = 17,
	_GT = 18,
	_GE = 19,
	_LODB = 20,
	_LODW = 21,
	_LODR = 22,
	_LEA = 23,
	_CMP = 24,
	_TST = 25,
	_STOB = 26,
	_STOW = 27,
	_JMP = 28,
	_ADJSP = 29,
	_JSB = 30,
	_RSB = 31,
	_PSHR = 32,
	_POPR = 33,
	_PSHB = 34,
	_PSHW = 35,
	_PSHA = 36,
	_LAND = 37,
	_LOR = 38,
};

/*
 * Storage
 */

EXTERN int

#ifdef DYNAMIC
  *nametab,		/* Nametable */
  *idents,		/* Identifiers */
  *mac,			/* Macro entries */
  *litq,		/* Literal pool */
  *sw,			/* Cases in switch */
#else
  nametab[NAMEMAX],
  idents[IDMAX*ILAST],
  mac[MACMAX*MLAST],
  litq[LITMAX],
  sw[SWMAX*SLAST],
#endif

	argcid, argvid,		// hashvalues of reserved words
	swinx,			// Position in switch table
	csp,			// stackpointer seen from scope coding
	hier_str[30],		// Array containing hierarchical operators
	hier_oper[30],		// Internal translation of the above
	idinx,			// Next free identifier
	inplnr,			// Linenumber of .C file
	inclnr,			// Linenumber of .H file
	ccode,			// True for C source, else ASM source
	macinx,			// Next free entry in mac
	macqinx,		// Next free entry in macq
	pinx,			// Position in preprocessor buffer
	iflevel,		// #if nesting level
	skiplevel,		// level at which #if skipping starts
	errflag,		// True if an error has occurred
	verbose,		// Verbose -v specified
	maklis,			// Listing -h specified
	outhdl,			// handle for .ASM file
	lishdl,			// handle for .LIS file
	inphdl,			// handle for .C file
	inchdl,			// handle for .H file
	reguse,			// Currently used registers
	regsum,			// Summary of all used registers
	reglock,		// Register locked by 'register' vars
	nxtlabel,		// Next label number
	currseg,		// Current output segment
	prevseg,		// Previous output segment
	litinx;			// Index to next entry

EXTERN char

#ifdef DYNAMIC
  *sbuf,		/* Source buffer */
  *pbuf,		/* Preprocessor buffer */
  *namech,		/* Nametable */
  *macq,		/* Macro string buffer */
#else
  sbuf[SBUFMAX],
  pbuf[PBUFMAX],
  namech[NAMEMAX],
  macq[MACQMAX],
#endif

	inpfn[PATHMAX],		// input filename
	incfn[PATHMAX],		// include filename
	outfn[PATHMAX],		// output filename
	*line,			// Pointer to current input buffer
	*lptr,			// Pointer to current character in input buffer
	ch,			// Current character in line being scanned
	nch;			// Next character in line being scanned
