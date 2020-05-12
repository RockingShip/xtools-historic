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
** xar.h -- Symbol definitions for X-ObjectArchiver
*/

/* 
** Compiler dependent parameters
*/

#define VERSION "X-Archiver, Version 1.0"

/* #define DYNAMIC		/* allocate memory dynamically */
/* #define UPPER		/* force symbols to uppercase */

#define NAMEMAX 2003		/* Size of nametable !!! MUST BE PRIME !!! */
#define FILEMAX 50		/* Size of filetable */

/*
** Machine dependent parmeters
*/

#define BPW		2  /* Bytes per word */

/*
** Definitions for names's
*/

#define NTAB		0
#define NCHAR		1
#define NLIB		2
#define NLAST		3

/*
** Definitions for files's
*/

#define FNAME		0
#define FLENGTH		1
#define FOFFSET		2
#define FOLDOFS		3
#define FLAST		4

/*
** Definitions for .OLB header
*/

#define HNAME		0
#define HFILE		1
#define HLAST		2

/*
** User commands
*/

#define ADDCMD		1
#define DELCMD		2
#define CRECMD		3
#define LISCMD		4
#define EXTCMD		5

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
  *file,		/* Filetable */
#else
  name[NAMEMAX*NLAST],
  file[FILEMAX*NLAST],
#endif

  datlen,		/* length of datbuf */
  olbhdr[HLAST],	/* .OLB header */
  usercmd,		/* user command */
  monitor,		/* Monitor -m specified */
  debug,		/* Debug   -d specified */
  olbhdl,		/* handle for source .OLB file */
  outhdl,		/* handle for destination .OLB file */
  objhdl;		/* handle for .OBJ file */

EXTERN char

  datbuf[512],		/* internal scratch buffer */
  modn[40],		/* name of moule */
  olbfn[40],		/* source .OLB filename */
  outfn[40],		/* destination .OLB filename */
  bakfn[40],		/* .BAK filename */
  objfn[40];		/* .OBJ filename */
