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
** xcc.h -- Symbol definitions for X-C-Compiler
*/

/* 
** Compiler dependent parameters
*/

/* #define DYNAMIC		/* allocate memory dynamically */
/* #define UPPER		/* force symbols to uppercase */

#define NAMEMAX 997		/* Size of nametable !!! MUST BE PRIME !!! */
#define PBUFMAX 512		/* Size of preprocessor buffer */
#define SBUFMAX 256		/* Size of source buffer */
#define MACMAX 300		/* Number of definable macro's */
#define MACQMAX MACMAX*7	/* Expansiontable for macro's */
#define GLBMAX 200		/* Number of global symbols */	
#define LOCMAX 50		/* Number of local symbols */	
#define SWMAX 100		/* Number of switch cases */	
#define PATHMAX 80              /* Length of filename */

/*
** Machine dependent parmeters
*/

#define BPW		2  /* Bytes per word */
#define SBIT            15 /* Bit number of signed bit */

/*
** Reserved characters 
*/

#define BS		8
#define HT		9
#define NEWLINE		10
#define FF		12

/*
** Possible values for "TYPE"
*/

#define CONSTANT	1
#define VARIABLE	2
#define ARRAY		3 /* converted by code generator to VARIABLE */
#define LABEL		4
#define FUNCTION	5
#define EXPR		6
#define BRANCH		7

/*
** Possible values for "CLASS"
*/

#define STATIC		1	/* static global variables */
#define SP_AUTO		2	/* local variables */
#define AP_AUTO		3	/* procedure parameters */
#define EXTERNAL	4	/* external global variables */
#define AUTOEXT		5	/* referenced names */
#define GLOBAL		6	/* global variables */
#define REGISTER	7	/* register variables */

/*
** Possible values for "EA"
*/

#define EA_ADDR		1
#define EA_IND		2
#define EA_REG		3

/*
** Definitions for lval  (Code generated node)
*/

#define LTYPE		0
#define LPTR		1
#define LSIZE		2
#define LEA		3
#define LNAME		4
#define LVALUE		5
#define LREG1		6
#define LREG2		7
#define LFALSE		8
#define LTRUE		9
#define LLAST		10

/*
** Definitions for ident (symbol table entry)
*/

#define INAME		0
#define ITYPE		1
#define IPTR		2
#define ICLASS		3
#define IVALUE		4
#define ISIZE		5
#define ILAST		6

/*
** Definitions for macro's
*/

#define MNAME		0
#define MEXPAND		1
#define MLAST		2

/*
** Definitions for switches
*/

#define SCASE		0
#define SLABEL		1
#define SLAST		2

/*
** Reserved registers 
*/

#define REG_SP		15
#define REG_AP		14
#define REG_BPW		13
#define REG_4		12
#define REG_1		11
#define REG_0		10
#define REG_RESVD	((1<<REG_SP)|(1<<REG_AP)|(1<<REG_BPW)|(1<<REG_4)|(1<<REG_1)|(1<<REG_0)|(1<<1)|(1<<0))

/*
** segment names 
*/

#define CODESEG		1
#define DATASEG		2
#define UDEFSEG		3

/*
** Compiler-generated p-codes
*/

#define _ILLEGAL	1
#define _ADD		2
#define _SUB		3
#define _MUL		4
#define _DIV		5
#define _MOD		6
#define _BOR		7
#define _XOR		8
#define _BAND		9
#define _LSR		10
#define _LSL		11
#define _NEG		12
#define _NOT		13
#define _EQ		14
#define _NE		15
#define _LT		16
#define _LE		17
#define _GT		18
#define _GE		19
#define _LODB		20
#define _LODW		21
#define _LODR		22
#define _LEA		23
#define _CMP		24
#define _TST		25
#define _STOB		26
#define _STOW		27
#define _JMP		28
#define _ADJSP		29
#define _JSB		30
#define _RSB		31
#define _PSHR		32
#define _POPR		33
#define _PSHB		34
#define _PSHW		35
#define _PSHA		36
#define _LAND		37
#define _LOR		38

/* 
** Literal pool
*/

#define LITMAX		1500

/*
** Storage
*/

EXTERN int 

#ifdef DYNAMIC
  *nametab,		/* Nametable */
  *mac,			/* Macro entries */
  *litq,		/* Literal pool */
  *glbsym,		/* Global symbols */
  *locsym,		/* Local symbols */
  *sw,			/* Cases in switch */
#else
  nametab[NAMEMAX],
  mac[MACMAX*MLAST], 
  litq[LITMAX],
  glbsym[GLBMAX*ILAST],
  locsym[LOCMAX*ILAST],
  sw[SWMAX*SLAST],
#endif

  argcid,argvid,	/* hashvalues of reserved words */
  swinx,                /* Position in switch table */
  csp,                  /* stackpointer seen from scope coding */
  hier_str[30],		/* Array containing hierichal operators */
  hier_oper[30],	/* Internal translation of the above */
  locinx,		/* Next free local symbol */
  glbinx,		/* Next free global symbol */
  inplnr,		/* Linenumber of .C file */
  inclnr,		/* Linenumber of .H file */
  ccode,		/* True for C source, else ASM source */
  macinx,		/* Next free entry in mac */
  macqinx,		/* Next free entry in macq */
  pinx,			/* Position in preprocessor buffer */
  iflevel,		/* #if nesting level */
  skiplevel,		/* level at which #if skipping starts */
  errflag,		/* True if an error has occurred */
  verbose,		/* Verbose -v specified */
  maklis,		/* Listing -h specified */
  outhdl,		/* handle for .ASM file */
  lishdl, 		/* handle for .LIS file */
  inphdl,		/* handle for .C file */
  inchdl,		/* handle for .H file */
  reguse,		/* Currently used registers */
  regsum,		/* Summary of all used registers */
  reglock,		/* Register locked by 'register' vars */
  nxtlabel,		/* Next label number */
  currseg,		/* Current output segment */
  prevseg,		/* Previous output segment */
  litinx;		/* Index to next entry */

EXTERN char 

#ifdef DYNAMIC
  *sbuf,		/* Source buffer */
  *pbuf,		/* Preprocessor buffer */
  *namech,		/* Nametable */
  *macq,		/* Macro string buffer */
#else
  sbuf[SBUFMAX],
  pbuf[PBUFMAX],
  namech[NAMEMAX],
  macq[MACQMAX],
#endif

  inpfn[PATHMAX],	/* input filename */
  incfn[PATHMAX],	/* include filename */
  outfn[PATHMAX],	/* output filename */
  *line,		/* Pointer to current input buffer */
  *lptr,		/* Pointer to current character in input buffer */
  ch,			/* Current character in line being scanned */
  nch;			/* Next character in line being scanned */
