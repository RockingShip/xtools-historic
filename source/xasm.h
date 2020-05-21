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

enum {
	NAMEMAX = 2003,		// Size of nametable !!! MUST BE PRIME !!!
	PBUFMAX = 512,		// Size of preprocessor buffer
	SBUFMAX = 256,		// Size of source buffer
	MACMAX = 300,		// Number of definable macro's
	MACQMAX = MACMAX * 7,	// Expansiontable for macro's
	PATHMAX = 80,		// Length of filename
	LITMAX = 1500,		// Literal pool
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
	OPC_ILLEGAL = 0x00,
	OPC_ADD = 0x13,
	OPC_SUB = 0x12,
	OPC_MUL = 0x14,
	OPC_DIV = 0x15,
	OPC_MOD = 0x16,
	OPC_OR = 0x17,
	OPC_XOR = 0x19,
	OPC_AND = 0x18,
	OPC_LSR = 0x1B,
	OPC_LSL = 0x1A,
	OPC_NEG = 0x1D,
	OPC_NOT = 0x1C,
	OPC_BEQ = 0x68,
	OPC_BNE = 0x67,
	OPC_BLT = 0x64,
	OPC_BLE = 0x63,
	OPC_BGT = 0x66,
	OPC_BGE = 0x65,
	OPC_LDB = 0x04,
	OPC_LDW = 0x01,
	OPC_LDR = 0x11,
	OPC_LEA = 0x03,
	OPC_CMP = 0x10,
	OPC_STB = 0x05,
	OPC_STW = 0x02,
	OPC_JMP = 0x6F,
	OPC_JSB = 0x20,
	OPC_RSB = 0x21,
	OPC_PSHR = 0x23,
	OPC_POPR = 0x24,
	OPC_PSHB = 0x25,
	OPC_PSHW = 0x26,
	OPC_PSHA = 0x27,
	OPC_SVC = 0x0A,
};

/*
 * Pseudo opcodes
 */

enum {
	PSEUDO_CODE = 1,
	PSEUDO_DATA = 2,
	PSEUDO_UDEF = 3,
	PSEUDO_ORG = 4,
	PSEUDO_END = 5,
	PSEUDO_DCB = 6,
	PSEUDO_DCW = 7,
	PSEUDO_DSB = 8,
	PSEUDO_DSW = 9,
};

/*
 * Object commands
 */

enum {
	REL_ADD = 1,
	REL_SUB = 2,
	REL_MUL = 3,
	REL_DIV = 4,
	REL_MOD = 5,
	REL_LSR = 6,
	REL_LSL = 7,
	REL_XOR = 8,
	REL_AND = 9,
	REL_OR = 10,
	REL_NOT = 11,
	REL_NEG = 12,
	REL_SWAP = 13,

	REL_END = 32,
	REL_SYMBOL = 33,
	REL_PUSHB = 34,
	REL_PUSHW = 35,
	REL_POPW = 36,
	REL_POPB = 37,
	REL_DSB = 38,

	REL_CODEB = 64,
	REL_CODEW = 65,
	REL_CODEDEF = 66,
	REL_CODEORG = 67,
	REL_DATAB = 72,
	REL_DATAW = 73,
	REL_DATADEF = 74,
	REL_DATAORG = 75,
	REL_UDEFB = 80,
	REL_UDEFW = 81,
	REL_UDEFDEF = 82,
	REL_UDEFORG = 83,
};

/*
 * Storage
 */

EXTERN int
	curpos[4],		// Position in segment
	curseg,			// Current segment
	datlen,			// length of data in datbuf
	debug,			// Debug   -d specified
	errflag,		// True if an error has occurred
	hier_oper[20],		// Internal translation of the above
	hier_str[20],		// Array containing hierarchical operators
	iflevel,		// #if nesting level
	inchdl,			// handle for .H   file
	inclnr,			// Linenumber of .H file
	inphdl,			// handle for .ASM file
	inplnr,			// Linenumber of .C file
	lishdl,			// handle for .LIS file
	litinx,			// Index to next entry
	litq[LITMAX],		// Temporary storage for initialisers
	mac[MACMAX * MLAST],	// macros
	macinx,			// Next free entry in mac
	macqinx,		// Next free entry in macq
	maxpos[4],		// Size in segment
	name[NAMEMAX * NLAST],	// string name table
	outhdl,			// handle for .OBJ file
	pass,			// Pass number
	pinx,			// Position in preprocessor buffer
	skiplevel,		// level at which #if skipping starts
	undef,			// Auto-external -u specified
	verbose;		// Verbose -v specified

EXTERN char
	ch,			// Current character in line being scanned
	datbuf[128],		// storage buffer for sto_data
	incfn[PATHMAX],		// include filename
	inpfn[PATHMAX],		// input filename
	*line,			// Pointer to current input buffer
	lisfn[PATHMAX],		// listing filename
	*lptr,			// Pointer to current character in input buffer
	macq[MACQMAX],
	nch,			// Next character in line being scanned
	outfn[PATHMAX],		// output filename
	pbuf[PBUFMAX],
	sbuf[SBUFMAX];
