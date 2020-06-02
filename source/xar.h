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
//* xar.h -- Symbol definitions for X-ObjectArchiver
//*

/* 
 * Internal storage limits
 */

enum {
	FILEMAX = 50,		// Size of filetable
	NAMEMAX = 2003,		// Size of nametable !!! MUST BE PRIME !!!
	PATHMAX = 80,		// length of filename
};

/*
 * Machine dependent parmeters
 */

enum {
	BPW = 2,		// Bytes per word
	SBIT = 15,		// Bit number of signed bit
};

/*
 * Definitions for names's
 */

enum {
	NTAB = 0,
	NCHAR,
	NLIB,
	NLAST,
};

/*
 * Definitions for files's
 */

enum {
	FNAME = 0,
	FLENGTH,
	FOFFSET,
	FOLDOFS,
	FLAST,
};

/*
 * Definitions for .OLB header
 */

enum {
	HNAME = 0,
	HFILE,
	HLAST,
};

/*
 * User commands
 */

enum {
	ADDCMD = 1,
	DELCMD,
	CRECMD,
	LISCMD,
	EXTCMD,
};

/*
 * Object commands
 */

enum {
	REL_END = 1,

	REL_ADD = 2,
	REL_SUB = 3,
	REL_MUL = 4,
	REL_DIV = 5,
	REL_MOD = 6,
	REL_LSR = 7,
	REL_LSL = 8,
	REL_XOR = 9,
	REL_AND = 10,
	REL_OR = 11,
	REL_NOT = 12,
	REL_NEG = 13,
	REL_SWAP = 14,

	REL_CODEB = 16,
	REL_CODEW = 17,
	REL_CODEDEF = 18,
	REL_CODEORG = 19,
	REL_DATAB = 20,
	REL_DATAW = 21,
	REL_DATADEF = 22,
	REL_DATAORG = 23,
	REL_TEXTB = 24,
	REL_TEXTW = 25,
	REL_TEXTDEF = 26,
	REL_TEXTORG = 27,
	REL_UDEFB = 28,
	REL_UDEFW = 29,
	REL_UDEFDEF = 30,
	REL_UDEFORG = 31,

	REL_SYMBOL = 32,
	REL_PUSHB = 33,
	REL_PUSHW = 34,
	REL_POPW = 35,
	REL_POPB = 36,
	REL_DSB = 37,
};

/*
 * Storage
 */

EXTERN int
	datlen,			// length of datbuf
	debug,			// Debug   -d specified
	errflag,		// Error occurred
	file[FILEMAX*NLAST],
	name[NAMEMAX*NLAST],
	objhdl,			// handle for .OBJ file
	olbhdl,			// handle for source .OLB file
	olbhdr[HLAST],		// .OLB header
	outhdl,			// handle for destination .OLB file
	usercmd,		// user command
	verbose;		// Verbose -v specified

EXTERN char
	bakfn[PATHMAX],		// .BAK filename
	datbuf[512],		// internal scratch buffer
	modn[PATHMAX],		// name of module
	objfn[PATHMAX],		// .OBJ filename
	olbfn[PATHMAX],		// source .OLB filename
	outfn[PATHMAX];		// destination .OLB filename
