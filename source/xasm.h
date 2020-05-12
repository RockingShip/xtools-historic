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
** xasm.h -- Symbol definitions for X-Assembler
*/

/* 
** Compiler dependent parameters
*/

#define VERSION "X-Assembler, Version 1.0"

/* #define DYNAMIC		/* allocate memory dynamically */
/* #define UPPER		/* force symbols to uppercase */

#define NAMEMAX 2003		/* Size of nametable !!! MUST BE PRIME !!! */
#define PBUFMAX 512		/* Size of preprocessor buffer */
#define SBUFMAX 256		/* Size of source buffer */
#define MACMAX 300		/* Number of definable macro's */
#define MACQMAX MACMAX*7	/* Expansiontable for macro's */

/*
** Machine dependent parmeters
*/

#define BPW		2  /* Bytes per word */

/* 
** Reserved characters 
*/

#define BS		8
#define HT		9
#define NEWLINE		10
#define FF		12

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
#define NVALUE		3
#define NLAST		4

/*
** Definitions for lval's
*/

#define LTYPE		0
#define LVALUE		1
#define LLAST		2

/*
** values for NTYPE
*/

#define UNDEF		 1
#define	ABS		 2
#define	CODE		 3
#define DATA		 4
#define UDEF		 5
#define OPCODE		 6
#define PSEUDO		 7
#define REGISTER	 8
#define LINK		 9
#define POINT		10

/*
** values for LTYPE
*/

#define CONSTANT	 1
#define SYMBOL		 2
#define	EXPRESSION	 3

/*
** Definitions for macro's
*/

#define MNAME		0
#define MEXPAND		1
#define MLAST		2

/*
** Compiler-generated p-codes
*/

#define _ILLEGAL	0x00
#define _ADD		0x13
#define _SUB		0x12
#define _MUL		0x14
#define _DIV		0x15
#define _MOD		0x16
#define _BOR		0x17
#define _XOR		0x19
#define _BAND		0x18
#define _LSR		0x1B
#define _LSL		0x1A
#define _NEG		0x1D
#define _NOT		0x1C
#define _EQ		0x68
#define _NE		0x67
#define _LT		0x64
#define _LE		0x63
#define _GT		0x66
#define _GE		0x65
#define _LODB		0x04
#define _LODW		0x01
#define _LODR		0x11
#define _LEA		0x03
#define _CMP		0x10
#define _STOB		0x05
#define _STOW		0x02
#define _JMP		0x6F
#define _JSB		0x20
#define _RSB		0x21
#define _PSHR		0x23
#define _POPR		0x24
#define _PSHB		0x25
#define _PSHW		0x26
#define _PSHA		0x27
#define _SVC		0x0A

/*
** Pseudo opcodes
*/

#define _CODE		1
#define _DATA		2
#define _UDEF		3
#define _ORG		4
#define _END		5
#define _DCB		6
#define _DCW		7
#define _DSB		8
#define _DSW		9

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
** Literal pool
*/

#define LITMAX		1500

/*
** Storage
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

  datlen,		/* length of data in datbuf */
  hier_str[20],		/* Array containing hierichal operators */
  hier_oper[20],	/* Internal translation of the above */
  pass,			/* Pass number */
  curseg,		/* Current segment */
  curpos[4],maxpos[4],	/* Position in segment */
  inplnr,		/* Linenumber of .C file */
  inclnr,		/* Linenumber of .H file */
  macinx,		/* Next free entry in mac */
  macqinx,		/* Next free entry in macq */
  pinx,			/* Position in preprocessor buffer */
  iflevel,		/* #if nesting level */
  skiplevel,		/* level at which #if skipping starts */
  errflag,		/* True if an error has occurred */
  undef,		/* Auto-external -u specified */
  monitor,		/* Monitor -m specified */
  pause,		/* Pause   -p specified */
  debug,		/* Debug   -d specified */
  maklis,		/* Listing -l specified */
  outhdl,		/* handle for .OBJ file */
  lishdl, 		/* handle for .LIS file */
  inphdl,		/* handle for .ASM file */
  inchdl,		/* handle for .H   file */
  litinx;		/* Index to next entry */

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

  datbuf[128],		/* storage buffer for sto_data */
  inpfn[40],		/* input filename */
  incfn[40],		/* include filename */
  outfn[40],		/* output filename */
  lisfn[40],		/* listing filename */
  *line,		/* Pointer to current input buffer */
  *lptr,		/* Pointer to current character in input buffer */
  ch,			/* Current character in line being scanned */
  nch;			/* Next character in line being scanned */
