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

enum {
	FILEMAX = 50,			// Number of files
	NAMEMAX = 2003,			// Size of nametable !!! MUST BE PRIME !!!
	PATHMAX = 80,			// Length of filename
	STACKMAX = 50,			// Size of linker stack
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
	DATASEG,
	TEXTSEG,
	UDEFSEG,
	LASTSEG,
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
	FDATAPOS,
	FDATALEN,
	FTEXTBASE,
	FTEXTPOS,
	FTEXTLEN,
	FUDEFBASE,
	FUDEFPOS,
	FUDEFLEN,
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
	TEXT,
	UDEF,
	ABS,
	UNDEF,
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
	curobj,			// index of current .OLB file
	curpos[LASTSEG],	// Position in segment
	curseg,			// Current segment
	datlen,			// length of data in datbuf
	debug,			// Debug   -d specified
	errflag,		// True if an error has occurred
	file1[FILEMAX*FLAST],	// files as arguments
	file1inx,		// Index to next entry
	file2[FILEMAX*FLAST],	// files processed
	file2inx,		// Index to next entry
	inphdl,			// handle for .OBJ/.OLB file
	lbfile[FILEMAX*LBFLAST],
	lbhdr[LBHLAST],		// .OLB header
	lbname[NAMEMAX*LBNLAST],// Library name table
	lishdl, 		// handle for .MAP file
	maxpos[LASTSEG],	// Size of segment
	name[NAMEMAX*NLAST],	// Name table
	outhdl,			// handle for .IMG file
	pass,			// Pass number
	stack[STACKMAX*BPW],	// REL evaluation stack
	stackinx,		// Poisition in stack
	stksiz,			// Stksiz  -s specified
	undef,			// Undef   -u specified
	verbose;		// Verbose -v specified

EXTERN char
	*line,			// Pointer to current input buffer
	*lptr,			// Pointer to current character in input buffer
	ch,			// Current character in line being scanned
	datbuf[512],		// storage buffer for sto_data
	inpfn[PATHMAX],		// input filename
	lisfn[PATHMAX],		// listing filename
	nch,			// Next character in line being scanned
	outfn[PATHMAX];		// output filename
