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
//* xlnk.h -- Symbol definitions for X-Linker
//*

/* 
 * Compiler dependent parameters
 */

// #define DYNAMIC		// allocate memory dynamically

enum {
	NAMEMAX = 2003,			// Size of nametable !!! MUST BE PRIME !!!
	FILEMAX = 50,			// Number of files
	STACKMAX = 50,			// Size of linker stack
	PATHMAX = 80,			// Length of filename
};

/*
 * Machine dependent parmeters
 */

enum {
	BPW = 2,			// Bytes per word
	SBIT = 15,			// Bit number of signed bit
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
	NMODULE,
	NVALUE,
	NLAST,
};

/*
 * Definitions for files's
 */

enum {
	FFILE = 0,
	FLIB,
	FOFFSET,
	FCODEBASE,
	FCODELEN,
	FCODEPOS,
	FDATABASE,
	FDATALEN,
	FDATAPOS,
	FUDEFBASE,
	FUDEFLEN,
	FUDEFPOS,
	FLAST,
};

/*
 * Definitions for .OLB header
 */

enum {
	LBHNAME = 0,
	LBHFILE,
	LBHLAST,
};

/*
 * Definitions for .OLB filetable
 */

enum {
	LBFNAME = 0,
	LBFLENGTH,
	LBFOFFSET,
	LBFOLDOFS,
	LBFLAST,
};

/*
 * Definitions for .OLB nametable
 */

enum {
	LBNTAB = 0,
	LBNCHAR,
	LBNLIB,
	LBNLAST,
};

/*
 * values for NTYPE
 */

enum {
	CODE = 1,
	DATA,
	UDEF,
	ABS,
	UNDEF,
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
  *file,		/* Files */
  *stack,		/* Linker stack */
#else
  name[NAMEMAX*NLAST],
  file1[FILEMAX*FLAST],
  file2[FILEMAX*FLAST],
  lbname[NAMEMAX*LBNLAST],
  lbfile[FILEMAX*LBFLAST],
  stack[STACKMAX*BPW],
#endif

	lbhdr[LBHLAST],		// .OLB header
	stackinx,		// Poisition in stack
	datlen,			// length of data in datbuf
	pass,			// Pass number
	curseg,			// Current segment
	curpos[4],maxpos[4],	// Position in segment
	errflag,		// True if an error has occurred
	stksiz,			// Stksiz  -s specified
	undef,			// Undef   -u specified
	verbose,		// Verbose -v specified
	debug,			// Debug   -d specified
	outhdl,			// handle for .IMG file
	lishdl, 		// handle for .MAP file
	inphdl,			// handle for .OBJ/.OLB file
	curobj,			// index of current .OLB file
	file1inx,		// Index to next entry
	file2inx;		// Index to next entry

EXTERN char

#ifdef DYNAMIC
  *sbuf,		/* Source buffer */
  *pbuf,		/* Preprocessor buffer */
  *macq,		/* Macro string buffer */
#else
/*
  sbuf[SBUFMAX],
  pbuf[PBUFMAX],
  macq[MACQMAX],
*/
#endif

	datbuf[512],		// storage buffer for sto_data
	inpfn[PATHMAX],		// input filename
	outfn[PATHMAX],		// output filename
	lisfn[PATHMAX],		// listing filename
	*line,			// Pointer to current input buffer
	*lptr,			// Pointer to current character in input buffer
	ch,			// Current character in line being scanned
	nch;			// Next character in line being scanned
