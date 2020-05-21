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
 * Compiler dependent parameters
 */

// #define DYNAMIC		// allocate memory dynamically

enum {
	NAMEMAX = 2003,		// Size of nametable !!! MUST BE PRIME !!!
	FILEMAX = 50,		// Size of filetable
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

#ifdef DYNAMIC
  *name,		/* Nametable */
  *file,		/* Filetable */
#else
  name[NAMEMAX*NLAST],
  file[FILEMAX*NLAST],
#endif

	datlen,			// length of datbuf
	debug,			// Debug   -d specified
	errflag,		// Error occurred
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
