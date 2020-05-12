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

/*
** xlnk.h -- Symbol definitions for X-Linker
*/

/* 
** Compiler dependent parameters
*/

#define VERSION "X-Linker, Version 1.0"

/* #define DYNAMIC		/* allocate memory dynamically */
/* #define UPPER		/* force symbols to uppercase */

#define NAMEMAX 2003		/* Size of nametable !!! MUST BE PRIME !!! */
#define FILEMAX	50		/* Number of files */
#define STACKMAX 50		/* Size of linker stack */

/*
** Machine dependent parmeters
*/

#define BPW		2  /* Bytes per word */

/*
** segment names 
*/

#define CODESEG		1
#define DATASEG		2
#define UDEFSEG		3

/*
** Definitions for names's
*/

#define NTAB		0
#define NCHAR		1
#define NTYPE		2
#define NMODULE		3
#define NVALUE		4
#define NLAST		5

/*
** Definitions for files's
*/

#define FFILE		 0
#define FLIB		 1
#define FOFFSET		 2
#define FCODEBASE	 3
#define FCODELEN	 4
#define FCODEPOS	 5
#define FDATABASE	 6
#define FDATALEN	 7
#define FDATAPOS	 8
#define FUDEFBASE 	 9
#define FUDEFLEN	10
#define FUDEFPOS	11
#define FLAST		12

/*
** Definitions for .OLB header
*/

#define LBHNAME		0
#define LBHFILE		1
#define LBHLAST		2
/*
** Definitions for .OLB filetable
*/

#define LBFNAME		0
#define LBFLENGTH	1
#define LBFOFFSET	2
#define LBFOLDOFS	3
#define LBFLAST		4

/*
** Definitions for .OLB nametable
*/

#define LBNTAB		0
#define LBNCHAR		1
#define LBNLIB		2
#define LBNLAST		3

/*
** values for NTYPE
*/

#define	CODE		 1
#define DATA		 2
#define UDEF		 3
#define	ABS		 4
#define UNDEF		 5

/*
** Object commands 
*/

#define __ADD		 1
#define __SUB		 2
#define __MUL		 3
#define __DIV		 4
#define __MOD		 5
#define __LSR		 6
#define __LSL		 7
#define __XOR		 8
#define __AND		 9
#define __OR		10
#define __NOT		11
#define __NEG		12
#define __SWAP		13
#define __END		32
#define __SYMBOL	33
#define __PUSHB		34
#define __PUSHW		35
#define __POPW		36
#define __POPB		37
#define __DSB		38

#define __CODEB		64
#define __CODEW		65
#define __CODEDEF	66
#define __CODEORG	67
#define __DATAB		72
#define __DATAW		73
#define __DATADEF	74
#define __DATAORG	75
#define __UDEFB		80
#define __UDEFW		81
#define __UDEFDEF	82
#define __UDEFORG	83

/*
** Storage
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

  lbhdr[LBHLAST],	/* .OLB header */
  stackinx,		/* Poisition in stack */
  datlen,		/* length of data in datbuf */
  pass,			/* Pass number */
  curseg,		/* Current segment */
  curpos[4],maxpos[4],	/* Position in segment */
  errflag,		/* True if an error has occured */
  stksiz,		/* Stksiz  -s specified */
  undef,		/* Undef   -u specified */
  monitor,		/* Monitor -m specified */
  debug,		/* Debug   -d specified */
  maklis,		/* Listing -l specified */
  outhdl,		/* handle for .IMG file */
  lishdl, 		/* handle for .MAP file */
  inphdl,		/* handle for .OBJ/.OLB file */
  curobj,		/* index of current .OLB file */
  file1inx,		/* Index to next entry */
  file2inx;		/* Index to next entry */

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

  datbuf[512],		/* storage buffer for sto_data */
  inpfn[40],		/* input filename */
  outfn[40],		/* output filename */
  lisfn[40],		/* listing filename */
  *line,		/* Pointer to current input buffer */
  *lptr,		/* Pointer to current character in input buffer */
  ch,			/* Current character in line being scanned */
  nch;			/* Next character in line being scanned */
