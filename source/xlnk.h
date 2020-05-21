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

	curobj,			// index of current .OLB file
	curpos[4],		// Position in segment
	curseg,			// Current segment
	datlen,			// length of data in datbuf
	debug,			// Debug   -d specified
	errflag,		// True if an error has occurred
	file1inx,		// Index to next entry
	file2inx,		// Index to next entry
	inphdl,			// handle for .OBJ/.OLB file
	lbhdr[LBHLAST],		// .OLB header
	lishdl, 		// handle for .MAP file
	maxpos[4],		// Size of segment
	outhdl,			// handle for .IMG file
	pass,			// Pass number
	stackinx,		// Poisition in stack
	stksiz,			// Stksiz  -s specified
	undef,			// Undef   -u specified
	verbose;		// Verbose -v specified

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

	*line,			// Pointer to current input buffer
	*lptr,			// Pointer to current character in input buffer
	ch,			// Current character in line being scanned
	datbuf[512],		// storage buffer for sto_data
	inpfn[PATHMAX],		// input filename
	lisfn[PATHMAX],		// listing filename
	nch,			// Next character in line being scanned
	outfn[PATHMAX];		// output filename
