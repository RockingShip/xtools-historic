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
  *file,		/* Filetable */
#else
  name[NAMEMAX*NLAST],
  file[FILEMAX*NLAST],
#endif

	datlen,			// length of datbuf
	olbhdr[HLAST],		// .OLB header
	usercmd,		// user command
	verbose,		// Verbose -v specified
	errflag,		// Error occurred
	debug,			// Debug   -d specified
	olbhdl,			// handle for source .OLB file
	outhdl,			// handle for destination .OLB file
	objhdl;			// handle for .OBJ file

EXTERN char

	datbuf[512],		// internal scratch buffer
	modn[PATHMAX],		// name of module
	olbfn[PATHMAX],		// source .OLB filename
	outfn[PATHMAX],		// destination .OLB filename
	bakfn[PATHMAX],		// .BAK filename
	objfn[PATHMAX];		// .OBJ filename
