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
//* xasm.h -- Symbol definitions for X-Assembler
//*

/* 
 * Compiler dependent parameters
 */

// #define DYNAMIC		// allocate memory dynamically

enum {
	NAMEMAX = 2003,         // Size of nametable !!! MUST BE PRIME !!!
	PBUFMAX = 512,          // Size of preprocessor buffer
	SBUFMAX = 256,          // Size of source buffer
	MACMAX = 300,           // Number of definable macro's
	MACQMAX = MACMAX * 7,   // Expansiontable for macro's
	PATHMAX = 80,           // Length of filename
	LITMAX = 1500,          // Literal pool
};

/*
 * Machine dependent parmeters
 */

enum {
	BPW = 2,                // Bytes per word
	SBIT = 15,              // Bit number of signed bit
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
 * segment names
 */

enum {
	CODESEG = 1,
	DATASEG = 2,
	UDEFSEG = 3,
};

/*
 * Definitions for names's
 */

enum {
	NTAB = 0,
	NCHAR,
	NTYPE,
	NVALUE,
	NLAST,
};

/*
 * Definitions for lval's
 */

enum {
	LTYPE = 0,
	LVALUE,
	LLAST,
};

/*
 * values for NTYPE
 */

enum {
	UNDEF = 1,
	ABS,
	CODE,
	DATA,
	UDEF,
	OPCODE,
	PSEUDO,
	REGISTER,
	LINK,
	POINT,
};

/*
 * values for LTYPE
 */

enum {
	CONSTANT = 1,
	SYMBOL,
	EXPRESSION,
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
 * Compiler-generated p-codes
 */

enum {
	_ILLEGAL = 0x00,
	_ADD = 0x13,
	_SUB = 0x12,
	_MUL = 0x14,
	_DIV = 0x15,
	_MOD = 0x16,
	_BOR = 0x17,
	_XOR = 0x19,
	_BAND = 0x18,
	_LSR = 0x1B,
	_LSL = 0x1A,
	_NEG = 0x1D,
	_NOT = 0x1C,
	_EQ = 0x68,
	_NE = 0x67,
	_LT = 0x64,
	_LE = 0x63,
	_GT = 0x66,
	_GE = 0x65,
	_LODB = 0x04,
	_LODW = 0x01,
	_LODR = 0x11,
	_LEA = 0x03,
	_CMP = 0x10,
	_STOB = 0x05,
	_STOW = 0x02,
	_JMP = 0x6F,
	_JSB = 0x20,
	_RSB = 0x21,
	_PSHR = 0x23,
	_POPR = 0x24,
	_PSHB = 0x25,
	_PSHW = 0x26,
	_PSHA = 0x27,
	_SVC = 0x0A,
};

/*
 * Pseudo opcodes
 */

enum {
	_CODE = 1,
	_DATA,
	_UDEF,
	_ORG,
	_END,
	_DCB,
	_DCW,
	_DSB,
	_DSW,
};

/*
 * Object commands
 */

enum {
	__ADD = 1,
	__SUB = 2,
	__MUL = 3,
	__DIV = 4,
	__MOD = 5,
	__LSR = 6,
	__LSL = 7,
	__XOR = 8,
	__AND = 9,
	__OR = 10,
	__NOT = 11,
	__NEG = 12,
	__SWAP = 13,
	__END = 32,
	__SYMBOL = 33,
	__PUSHB = 34,
	__PUSHW = 35,
	__POPW = 36,
	__POPB = 37,
	__DSB = 38,

	__CODEB = 64,
	__CODEW = 65,
	__CODEDEF = 66,
	__CODEORG = 67,
	__DATAB = 72,
	__DATAW = 73,
	__DATADEF = 74,
	__DATAORG = 75,
	__UDEFB = 80,
	__UDEFW = 81,
	__UDEFDEF = 82,
	__UDEFORG = 83,
};

/*
 * Storage
 */

EXTERN int 

#ifdef DYNAMIC
  *name,		/* Nametable */
  *mac,			/* Macro entries */
  *litq,		/* Literal pool */
#else
  name[NAMEMAX*NLAST],
  mac[MACMAX*MLAST], 
  litq[LITMAX],
#endif

  datlen,		// length of data in datbuf
  hier_str[20],		// Array containing hierarchical operators
  hier_oper[20],	// Internal translation of the above
  pass,			// Pass number
  curseg,		// Current segment
  curpos[4],maxpos[4],	// Position in segment
  inplnr,		// Linenumber of .C file
  inclnr,		// Linenumber of .H file
  macinx,		// Next free entry in mac
  macqinx,		// Next free entry in macq
  pinx,			// Position in preprocessor buffer
  iflevel,		// #if nesting level
  skiplevel,		// level at which #if skipping starts
  errflag,		// True if an error has occurred
  undef,		// Auto-external -u specified
  verbose,		// Verbose -v specified
  debug,		// Debug   -d specified
  outhdl,		// handle for .OBJ file
  lishdl, 		// handle for .LIS file
  inphdl,		// handle for .ASM file
  inchdl,		// handle for .H   file
  litinx;		// Index to next entry

EXTERN char 

#ifdef DYNAMIC
  *sbuf,		/* Source buffer */
  *pbuf,		/* Preprocessor buffer */
  *macq,		/* Macro string buffer */
#else
  sbuf[SBUFMAX],
  pbuf[PBUFMAX],
  macq[MACQMAX],
#endif

  datbuf[128],		// storage buffer for sto_data
  inpfn[PATHMAX],	// input filename
  incfn[PATHMAX],	// include filename
  outfn[PATHMAX],	// output filename
  lisfn[PATHMAX],	// listing filename
  *line,		// Pointer to current input buffer
  *lptr,		// Pointer to current character in input buffer
  ch,			// Current character in line being scanned
  nch;			// Next character in line being scanned
