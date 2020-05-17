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
//* X-Assembler.  Part 1, I/O Support
//*

#define EXTERN
#include "xasm.h"

//*
//*
//* Compiler startup routines
//*
//*

/*
 * Initialize all variables
 */
add_res (opc, typ, val)
char *opc;
int typ, val;
{
char buf[20];
int hash;
register int *p;
register char *cp1, *cp2;

  // upper case
  dohash (opc, &hash);
  p = &name[hash*NLAST];
  p[NTYPE] = typ;
  p[NVALUE] = val;

  // AND lowercase
  cp1 = opc;
  cp2 = buf;
  while (*cp1)
    *cp2++ = ((*cp1 >= 'A') && (*cp1 <= 'Z')) ? *cp1++ -'A'+'a' : *cp1++; 
  *cp2++ = 0;
  dohash (buf, &hash);
  p = &name[hash*NLAST];
  p[NTYPE] = typ;
  p[NVALUE] = val;
}

/*
 *
 */
initialize ()
{
register int i,*p;

  verbose = debug = 0;
  outhdl = lishdl = inphdl = inchdl = 0;
  iflevel = skiplevel = 0;
  macinx = macqinx = 0;
  inclnr = inplnr = 0;
  inpfn[0] = 0;
  datlen = 0;

#ifdef DYNAMIC
  ! check this first
  litq = calloc (LITMAX*BPW);
  name = calloc (NAMEMAX*BPW*NLAST);
  sbuf = calloc (SBUFMAX);
  pbuf = calloc (PBUFMAX);
  mac = calloc (MACMAX*BPW*MLAST);
  macq = calloc (MACQMAX);
  glbsym = calloc (GLBMAX*BPW*ILAST);
  locsym = calloc (LOCMAX*BPW*ILAST);
#endif;

  // reset table
  for (i=0; i<NAMEMAX; i++) {
    p = &name[i*NLAST];
    p[NCHAR] = p[NTYPE] = 0;
  }

  // reserve first entry so it terminates lists
  name[0*NLAST+NCHAR] = '?';

  // reset positions
  pass = 1;
  curseg = CODESEG;
  curpos[CODESEG] = curpos[DATASEG] = curpos[UDEFSEG] = 0;
  maxpos[CODESEG] = maxpos[DATASEG] = maxpos[UDEFSEG] = 0;

  // reserved words
  add_res ("ILLEGAL",	OPCODE, _ILLEGAL);
  add_res ("ADD",	OPCODE, _ADD);
  add_res ("SUB",	OPCODE, _SUB);
  add_res ("MUL",	OPCODE, _MUL);
  add_res ("DIV",	OPCODE, _DIV);
  add_res ("MOD",	OPCODE, _MOD);
  add_res ("OR",	OPCODE, _BOR);
  add_res ("XOR",	OPCODE, _XOR);
  add_res ("AND",	OPCODE, _BAND);
  add_res ("LSR",	OPCODE, _LSR);
  add_res ("LSL",	OPCODE, _LSL);
  add_res ("NEG",	OPCODE, _NEG);
  add_res ("NOT",	OPCODE, _NOT);
  add_res ("BEQ",	OPCODE, _EQ);
  add_res ("BNE",	OPCODE, _NE);
  add_res ("BLT",	OPCODE, _LT);
  add_res ("BLE",	OPCODE, _LE);
  add_res ("BGT",	OPCODE, _GT);
  add_res ("BGE",	OPCODE, _GE);
  add_res ("LDB",	OPCODE, _LODB);
  add_res ("LDW",	OPCODE, _LODW);
  add_res ("LDR",	OPCODE, _LODR);
  add_res ("LDA",	OPCODE, _LEA);
  add_res ("CMP",	OPCODE, _CMP);
  add_res ("STB",	OPCODE, _STOB);
  add_res ("STW",	OPCODE, _STOW);
  add_res ("JMP",	OPCODE, _JMP);
  add_res ("JSB",	OPCODE, _JSB);
  add_res ("RSB",	OPCODE, _RSB);
  add_res ("PSHR",	OPCODE, _PSHR);
  add_res ("POPR",	OPCODE, _POPR);
  add_res ("PSHB",	OPCODE, _PSHB);
  add_res ("PSHW",	OPCODE, _PSHW);
  add_res ("PSHA",	OPCODE, _PSHA);
  add_res ("SVC",	OPCODE, _SVC);
  add_res (".CODE",	PSEUDO, _CODE);
  add_res (".DATA",	PSEUDO, _DATA);
  add_res (".UDEF",	PSEUDO, _UDEF);
  add_res (".ORG",	PSEUDO, _ORG);
  add_res (".END",	PSEUDO, _END);
  add_res (".DCB",	PSEUDO, _DCB);
  add_res (".DCW",	PSEUDO, _DCW);
  add_res (".DSB",	PSEUDO, _DSB);
  add_res (".DSW",	PSEUDO, _DSW);
  add_res ("R0",	REGISTER,  0);
  add_res ("R1",	REGISTER,  1);
  add_res ("R2",	REGISTER,  2);
  add_res ("R3",	REGISTER,  3);
  add_res ("R4",	REGISTER,  4);
  add_res ("R5",	REGISTER,  5);
  add_res ("R6",	REGISTER,  6);
  add_res ("R7",	REGISTER,  7);
  add_res ("R8",	REGISTER,  8);
  add_res ("R9",	REGISTER,  9);
  add_res ("R10",	REGISTER, 10);
  add_res ("R11",	REGISTER, 11);
  add_res ("R12",	REGISTER, 12);
  add_res ("R13",	REGISTER, 13);
  add_res ("R14",	REGISTER, 14);
  add_res ("R15",	REGISTER, 15);
  add_res (".",		POINT, 0);

  hier_str[ 0] = "|";  hier_oper[ 0] = __OR;    // hier1
  hier_str[ 1] = 0;                                   
  hier_str[ 2] = "^";  hier_oper[ 2] = __XOR;   // hier2
  hier_str[ 3] = 0;                                   
  hier_str[ 4] = "&";  hier_oper[ 4] = __AND;   // hier3
  hier_str[ 5] = 0;                                   
  hier_str[ 6] = ">>"; hier_oper[ 6] = __LSR;   // hier4
  hier_str[ 7] = "<<"; hier_oper[ 7] = __LSL;            
  hier_str[ 8] = 0;                                   
  hier_str[ 9] = "+";  hier_oper[ 9] = __ADD;   // hier5
  hier_str[10] = "-";  hier_oper[10] = __SUB;            
  hier_str[11] = 0;                                   
  hier_str[12] = "*";  hier_oper[12] = __MUL;   // hier6
  hier_str[13] = "/";  hier_oper[13] = __DIV;            
  hier_str[14] = "%";  hier_oper[14] = __MOD;            
  hier_str[15] = 0;
}

/*
 * Process commandline
 */
usage ()
{
  printf ("X-Assembler, Version %s\n\n", getversion());

  printf ("usage: xasm <file>[.<ext>]\n");
  printf ("  -a <file>[.<ext>]]\tListing\n");
  printf ("  -c <file>[.<ext>]]\tObject output\n");
  printf ("  -v\t\t\tVerbose\n");
  exit (1);
}

fext(out, path, ext, force)
char *out;
char *path;
char *ext;
int force;
{
  char *p;
  int  baselen;

  baselen = 0;
  for (p  = path; *p; p++) {
    if (*p == '\\' || *p == '/')
      baselen = 0;
    else if (*p == '.')
      baselen = p - path;
  }

  if (baselen && !force)
    strcpy(out, path);
  else {
    if (!baselen)
      baselen = p - path;
    strncpy(out, path, baselen);
    strcpy(out + baselen, ext);
  }
}

startup (argv)
register int *argv;
{
  argv++; // skip argv[0]
  while (*argv) {
    register char *arg;
    arg = *argv++;

    if (*arg != '-') {
      fext(inpfn, arg, ".xs", 0);
      if (!outfn[0])
        fext(outfn, arg, ".xo", 1);
    } else {
      // Process option
      arg++;
      switch (*arg++) {
	case 'a':
	  if (!*arg && *argv)
	    arg = *argv++;
	  if (!*arg || *arg == '-')
	    fext(lisfn, inpfn, ".lis", 1);
	  else
            fext(lisfn, arg, ".lis", 0);
	  break;
	case 'c':
	  if (!*arg && *argv)
	    arg = *argv++;
	  if (!*arg || *arg == '-')
	    usage();
          else
	    fext(outfn, arg, ".xo", 0);
	  break;
	case 'd':
  	  debug = 1;
	  break;
        case 'v':
          verbose = 1;
          break;
        default:
          usage ();
          break;
      }
    }
  }

  // filename MUST be supplied
  if (!outfn[0])
    usage ();
}

/*
 * Open all files
 */
mustopen(fn, mode)
char *fn;
char *mode;
{
int fd;

  fd=fopen(fn, mode);
  if (fd > 0)
    return fd;
  printf ("fopen(%s,%s) failed\n", fn, mode);
  exit (1);
}

openfile ()
{
  inphdl = mustopen (inpfn, "r");
  outhdl = mustopen (outfn, "w");
  if (lisfn[0])
    lishdl = mustopen (lisfn, "w");
}

//*
//*
//* Input support routines
//*
//*

/*
 * Get next character from current line
 */
gch ()
{
register int c;

  if (c=ch)
    bump(1);
  return c;
}

/*
 * Erase current line
 */
kill ()
{
  *line = 0;
  bump(0);
}

/*
 * Bump next characters in line (n = #chars or 0 for initialization)
 */
bump (n)
register int n;
{
  lptr = n ? lptr+n : line;
  nch = (ch = lptr[0]) ? lptr[1] : 0;
}

//*
//*
//* Symboltable routines
//*
//*

foutname (hash)
register int hash;
{
register int i;

  i=name[hash*NLAST+NTAB];
  if (i)
    foutname (i);
  fprintf (outhdl, "%c", name[hash*NLAST+NCHAR]); 
}

outname (hash)
register int hash;
{
register int i;

  i=name[hash*NLAST+NTAB];
  if (i)
    outname (i);
  printf ("%c", name[hash*NLAST+NCHAR]); 
}

lenname (hash)
register int hash;
{
register int i;

  i=name[hash*NLAST+NTAB];
  if (i)
    return lenname (i) + 1;
  return 1;
}

/*
 * Get the (unique) hashed value for symbol, return length
 */
dohash (ident, retval)
register char *ident;
int *retval;
{
register int start, hash, tab, len, *p;

  if (!alpha (*ident))
    return 0; // Not a symbol

  tab = 0;
  len = 0;
  hash = 0;
  while (an (*ident)) {
    start = hash = (hash + *ident * *ident) % NAMEMAX;
    while (1) {
      p = &name[hash*NLAST];
      if ((p[NCHAR] == *ident) && (p[NTAB] == tab)) {
        tab = hash;
        break; // Inner loop
      } else if (!p[NCHAR]) {
        p[NCHAR] = *ident;
        p[NTAB] = tab;
        tab = hash;
        break; // Inner loop
      } else {
        hash += *ident;
        if (hash >= NAMEMAX)
          hash -= NAMEMAX;
        if (hash == start)
          fatal ("name table overflow");
      }
    }
    ++ident;
    ++len;
  }
  *retval = tab;
  return len;
}

//*
//*
//* Preprocessor
//*
//*

/*
 * open an include file
 */
doinclude ()
{
register char *p;

  white ();
  if (*lptr != '"')
    error ("filename expected");
  else if (inchdl)
    error("Nested #include not allowed");
  else {
    // Modify sourceline to extract filename
    p = incfn;
    gch (); // Skip delimiter
    while ((*p++ = gch ()) != '"') ;
    *--p = 0; // Remove delimiter
    
    // Open file
    inchdl = mustopen (incfn, "r");
    }

  // make next read come from new file (if open)
  kill ();
}

/*
 * Declare/define a macro
 */
declmac ()
{
int sname;
register int len;
register int *mptr;

  white ();
  if (!(len = dohash (lptr, &sname)))
    error ("identifier expected");
  else if (macinx >= MACMAX)
    fatal ("#define overflow");
  else {
    // copy macroname
    mptr = &mac[macinx++ * MLAST];
    mptr[MNAME] = sname; 
    mptr[MEXPAND] = &macq[macqinx];
    // Copy expansion
    bump (len);
    white ();
    while (ch) {
      if (macqinx < MACQMAX)
        macq[macqinx++] = ch;
      gch ();
    }
    if (macqinx < MACQMAX)
      macq[macqinx++] = 0; // Terminator
    if (macqinx >= MACQMAX)
      fatal ("#define string overflow");
  }
}

findmac (sname)
register int sname;
{
register int i;
register int *mptr;

  for (i=0; i<macinx; i++) {
    mptr = &mac[i*MLAST];
    if (mptr[MNAME] == sname)
      return mptr;
  }
  return 0;
}

keepch (c)
int c;
{
  if (pinx < PBUFMAX)
    pbuf[pinx++] = c;
}

readline ()
{
  *sbuf = 0;
  if (inphdl)
    while (!*sbuf) {
      if (inchdl) {
        if (!fgets (sbuf, SBUFMAX, inchdl)) {
          fclose (inchdl);
          inchdl = 0;
          continue;
        }
        ++inclnr;
      } else if (inphdl) {
        if (!fgets (sbuf, SBUFMAX, inphdl)) {
          fclose (inphdl);
          inphdl = 0;
          break;
        }
        ++inplnr;
      }
    }

  // Make buffer available
  line = sbuf;
  bump (0);
}

ifline () 
{
int sname;

  while (1) {
    readline ();
    if (!inphdl)
      break;
    white ();
    if (!ch)
      continue; // Try again

    if (amatch ("#ifdef")) {
      ++iflevel;
      if (!skiplevel) {
        white ();
        if (!dohash (lptr, &sname))
          error ("identifier expected");
        else if (!findmac (sname))
          skiplevel = iflevel;
      }
    } else if (amatch("#ifndef")) {
      ++iflevel;
      if (!skiplevel) {
        white ();
        if (!dohash (lptr, &sname))
          error ("identifier expected");
        else if (findmac (sname))
          skiplevel = iflevel;
      }
    } else if (amatch("#else")) {
      if (iflevel) {
        if (skiplevel == iflevel)
          skiplevel = 0;
        else if (!skiplevel)
          skiplevel = iflevel;
      } else 
        error("no matching #if...");
    } else if (amatch("#endif")) {
      if (iflevel) {
        if (skiplevel == iflevel)
          skiplevel = 0;
        --iflevel;
      } else
        error("no matching #if...");
    } else if (!skiplevel)
      return 0; // Process this line
  }
}

preprocess ()
{
register char *optr, *cptr;
register int i, len;
int *mptr;
int sname;

  // get inputline
  ifline ();
  if (!inphdl)
    return;

  // Now expand current line
  pinx = 0;
  while (ch) {
    if (ch <= ' ') {
      keepch (' ');
      while (ch && (ch <= ' '))
        gch ();
    } else if (ch == '"') {
      keepch (gch ());
      while (ch != '"') {
        if (ch == '\\') {
          keepch (gch());
          keepch (gch());
        } else if (!ch) {
          if (pass == 1)
            error ("no quote");
          break;
        } else 
          keepch (gch());
      }
      keepch ('\"');
      gch ();
    } else if (ch == '\'') {
      keepch (gch ());
      while (ch != '\'') {
        if (ch == '\\') {
          keepch (gch());
          keepch (gch());
        } else if (!ch) {
          if (pass == 1)
            error ("no apostrophe");
          break;
        } else 
          keepch (gch());
      }
      keepch ('\'');
      gch ();
    } else if (ch == ';') {
      break;
    } else if (len = dohash(lptr, &sname)) {
       if (mptr = findmac (sname)) {
         cptr = mptr[MEXPAND];
         while (i = *cptr++)
           keepch (i);
         bump (len);
       } else {
         while (len--)
           keepch (gch ());
       }
    } else
      keepch (gch ());
  }

  // make line available
  keepch (0);
  if (pinx == PBUFMAX)
    error("line too long");
  line = pbuf;
  bump (0);

  // Copy line to listing
  if (lishdl) {
    if (curseg == CODESEG)
      fprintf (lishdl, ";C%04x: %s\n", curpos[CODESEG] & 0xffff, pbuf);
    else if (curseg == DATASEG)
      fprintf (lishdl, ";D%04x: %s\n", curpos[DATASEG] & 0xffff, pbuf);
    else
      fprintf (lishdl, ";U%04x: %s\n", curpos[UDEFSEG] & 0xffff, pbuf);
  }
}
 
//*
//*
//* Converts and Matches
//*
//*

/* 
 * Skip all spaces in current line
 */
white ()
{
  while (ch && (ch <= ' '))
    gch ();
}

/*
 * Convert a character to uppercase
 */
toupper (c)
register int c;
{
  return ((c >= 'a') && (c <= 'z')) ? c - 'a' + 'A' : c;
}

/*
 * Return 'true' if c is a decimal digit
 */
isdigit (c)
register int c;
{
  return ((c >= '0') && (c <= '9'));
}

/*
 * Return 'true' if c is a hexadecimal digit (0-9, A-F, or a-f)
 */
isxdigit (c)
register int c;
{
  return  ( ((c >= '0') && (c <= '9')) ||
            ((c >= 'a') && (c <= 'f')) ||
            ((c >= 'A') && (c <= 'F')) );
}

/*
 * Return 'true' if c is alphanumeric
 */
an (c)
register int c;
{
  return  ( ((c >= 'a') && (c <= 'z')) ||
            ((c >= 'A') && (c <= 'Z')) ||
            ((c >= '0') && (c <= '9')) ||
            (c == '_') || (c == '.')   );
}

/*
 * Return 'true' if c is alphabetic
 */
alpha (c)
register int c;
{
  return  ( ((c >= 'a') && (c <= 'z')) ||
            ((c >= 'A') && (c <= 'Z')) ||
            (c == '_') || (c == '.')   );
}

/*
 * Return 'index' if both strings match
 */
streq (str1,str2)
register char *str1,*str2;
{
register int i;

  i=0;
  while (str2[i]) {
    if (str1[i] != str2[i])
      return 0;
    i++;
  }
  return i;
}

/*
 * Return 'index' if str2 matches alphanumeric token str1
 */
astreq (str1,str2)
register char *str1,*str2;
{
register int i;

  i=0;
  while (str2[i]) {
    if (str1[i] != str2[i])
      return 0;
    i++;
  }
  if (an (str1[i]))
     return 0;
  return i;
}

/*
 * Return 'index' if start next token equals 'lit'
 */
match (lit)
char *lit;
{
register int i;

  if (lptr[0] <= ' ')
    white ();
  if (i=streq (lptr, lit)) {
    bump (i);
    return 1;
  }
  return 0;
}

/*
 * Return 'index' if next token equals 'lit'
 */
amatch (lit)
char *lit;
{
register int i;

  if (lptr[0] <= ' ')
    white ();
  if (i=astreq (lptr, lit)) {
    bump (i);
    return 1;
  }
  return 0;
}

/*
 * Return 'true' if next operator equals 'lit'
 */
omatch (lit)
register char *lit;
{
  if (lptr[0] <= ' ')
    white();
  if (lptr[0] != lit[0])
    return 0;
  if (lit[1]) {
    if (lptr[1] != lit[1])
      return 0;
    bump(2);
  } else {
    if ((lptr[1] == '=') || (lptr[1] == lptr[0]))
      return 0;
    bump(1);
  }
  return 1;
}

//*
//*
//* Error routines
//*
//*

/*
 * Generate error messages
 */
error(msg)
char *msg;
{
  if (inchdl)
    printf ("'%s' ", incfn);
  // Display original line
  printf ("%d: %s\n%%%s\n", inchdl ? inclnr : inplnr, sbuf, msg);
  if (lishdl)
    fprintf (lishdl, ";%d %%%s\n", inchdl ? inclnr : inplnr, msg);

  errflag = 1;
}

fatal (msg)
char *msg;
{
  error (msg);
  exit (1);
}

exprerr ()
{
  error ("Invalid expression");
  junk ();
}

needtoken(str)
char *str;
{
char txt[32], *p1, *p2;

  if (!match (str)) {
    p1 = txt; 
    p2 = "Expected "; 
    while (*p1++ = *p2++) ;
    --p1; // Overwrite terminator
    while (*p1++ = *str++) ;
    error(txt);
  }
}

/*
 * Skip current symbol
 */
junk ()
{
  if (an (ch)) {
    while (ch && an (ch))
      gch ();
  } else {
    while (ch && !an (ch))
      gch ();
  }
  white ();
}

/*
 * semicolon enforcer
 */
ns ()
{
  if (!match(";")) {
    error ("no semicolon");
    junk();
  } else 
    errflag = 0;
}

//*
//*
//* Main
//*
//*

/*
 * Execution starts here
 */
main (argc, argv)
int argc;
int *argv;
{
register int i, j, *p;

  initialize ();        // initialize all variables
  
  startup (argv);       // Process commandline options
  openfile ();          // Open all files
  preprocess ();        // fetch first line

  pass = 1;
  if (verbose)
    printf ("Pass 1\n");
  if (lishdl)
    fprintf (lishdl, "Pass 1\n");
  parse (); // GO !!!
  if (iflevel)
   error ("no closing #endif");
  
  if (!inphdl)
    printf ("%%missing .END statement\n");
  else
    fclose (inphdl);

  inphdl = mustopen (inpfn, "r");
  macinx = macqinx = 0;
  iflevel = skiplevel = 0;
  inclnr = inplnr = 0;
  savmaxpos ();
  curseg = CODESEG;
  curpos[CODESEG] = curpos[DATASEG] = curpos[UDEFSEG] = 0;
  datlen = 0;

  pass = 2;
  if (verbose)
    printf ("Pass 2\n");
  if (lishdl)
  fprintf (lishdl, "Pass 2\n");
  parse (); // GO !!!
  if (iflevel)
   error ("no closing #endif");

  sto_cmd (__END, 0);

  if (lishdl) {
    fprintf (lishdl, "CODE         : %04x (%5d)\n", maxpos[CODESEG] & 0xffff, maxpos[CODESEG] & 0xffff);
    fprintf (lishdl, "DATA         : %04x (%5d)\n", maxpos[DATASEG] & 0xffff, maxpos[DATASEG] & 0xffff);
    fprintf (lishdl, "UDEF         : %04x (%5d)\n", maxpos[UDEFSEG] & 0xffff, maxpos[UDEFSEG] & 0xffff);
    fprintf (lishdl, "Macros       : %5d(%5d)\n", macinx, MACMAX);
    j=0; for (i=0; i<NAMEMAX; i++) if (name[i*NLAST+NCHAR]) j++;
    fprintf (lishdl, "Names        : %5d(%5d)\n", j, NAMEMAX);
  }

  return errflag;
}

//*
//*
//* General output routines
//*
//*

write_byte(byte)
char byte;
{
  fwrite (&byte, 1, 1, outhdl);
}

write_word(word)
int word;
{
  char arr[2];
  arr[0] = word >> 8;
  arr[1] = word;

  fwrite (arr, 1, 2, outhdl);
}

sto_flush ()
{
char cval;
register int i,j;

  if ((pass == 2) && datlen) {
    if (!debug) {
      cval = -datlen;
      write_byte (cval);
      fwrite (datbuf, 1, datlen, outhdl);
    } else {
      i = 0;
      while (i < datlen) {
        for (j=0; j<16; j++) {
          fprintf (outhdl, "0x%02x", (i >= datlen) ? 0 : datbuf[i]);
          if (++i >= datlen)
            break;
          if (j != 15)
            fprintf (outhdl, ",");
        }
        fprintf (outhdl, "\n");
      }
    }

    datlen = 0;
  }
}

sto_data (val, size)
int val, size;
{
  if (size == BPW) {
    // store upper byte
    if (datlen == 128)
      sto_flush ();
    datbuf[datlen++] = val>>8;
  }
    // store lower byte
  if (datlen == 128)
    sto_flush ();
  datbuf[datlen++] = val;
}

sto_cmd (cmd, val)
char cmd;
int val;
{
char cval;
register int *p;

  sto_flush ();
  cval = val;
  if (pass == 2) {
    if (!debug) 
      switch (cmd) {
        case __ADD:  case __SUB:  case __MUL:  case __DIV: case __MOD:
        case __LSR:  case __LSL:  case __AND:  case __OR:  case __XOR:
        case __SWAP: case __POPW: case __POPB: case __END:
          write_byte (cmd);
          break;
        case __PUSHW: case __PUSHB:
          if ((val >= -128) && (val <= 127))
            cmd = __PUSHB;
          if (cmd == __PUSHB) {
            cval = val;
            write_byte (cmd);
            write_byte (cval);
          } else {
            write_byte (cmd);
            write_word (val);
          }
          break;
        case __CODEW: case __CODEB:
          if ((val >= -128) && (val <= 127))
            cmd = __CODEB;
          if (cmd == __CODEB) {
            write_byte (cmd);
            write_byte (cval);
          } else {
            write_byte (cmd);
            write_word (val);
          }
          break;
        case __DATAW: case __DATAB:
          if ((val >= -128) && (val <= 127))
            cmd = __DATAB;
          if (cmd == __DATAB) {
            write_byte (cmd);
            write_byte (cval);
          } else {
            write_byte (cmd);
            write_word (val);
          }
          break;
        case __UDEFW: case __UDEFB:
          if ((val >= -128) && (val <= 127))
            cmd = __UDEFB;
          if (cmd == __UDEFB) {
            write_byte (cmd);
            write_byte (cval);
          } else {
            write_byte (cmd);
            write_word (val);
          }
          break;
        case __SYMBOL:
          write_byte (cmd);
          write_byte (lenname (val));
          foutname (val);
          break;
        case __CODEDEF: case __DATADEF: case __UDEFDEF:
          p = &name[val*NLAST];
          write_byte (cmd);
          write_word (p[NVALUE]);
          cval = lenname (val);
          write_byte (cval);
          foutname (val);
          break;
        case __CODEORG: case __DATAORG: case __UDEFORG: 
        case __DSB:
          write_byte (cmd);
          write_word (val);
          break;
        default:   
          printf ("unimplemented OBJECT cmd: %d\n", cmd);
          exit (1);
          break;
     }
    else {
      switch (cmd) {
        case __ADD: fprintf (outhdl, "ADD\n"); break;
        case __SUB: fprintf (outhdl, "SUB\n"); break;
        case __MUL: fprintf (outhdl, "MUL\n"); break;
        case __DIV: fprintf (outhdl, "DIV\n"); break;
        case __MOD: fprintf (outhdl, "MOD\n"); break;
        case __LSR: fprintf (outhdl, "LSR\n"); break;
        case __LSL: fprintf (outhdl, "LSL\n"); break;
        case __AND: fprintf (outhdl, "AND\n"); break;
        case __OR:  fprintf (outhdl, "OR\n");  break;
        case __XOR: fprintf (outhdl, "XOR\n"); break;
        case __SWAP: fprintf (outhdl, "SWAP\n"); break;
        case __POPB: fprintf (outhdl, "POPB\n"); break;
        case __POPW: fprintf (outhdl, "POPW\n"); break;
	case __PUSHW: fprintf (outhdl, "PUSHW %d\n", val); break;
	case __PUSHB: fprintf (outhdl, "PUSHB %d\n", val); break;
	case __CODEW: fprintf (outhdl, "CODEW %d\n", val); break;
	case __CODEB: fprintf (outhdl, "CODEB %d\n", val); break;
	case __DATAW: fprintf (outhdl, "DATAW %d\n", val); break;
	case __DATAB: fprintf (outhdl, "DATAB %d\n", val); break;
	case __UDEFW: fprintf (outhdl, "UDEFW %d\n", val); break;
	case __UDEFB: fprintf (outhdl, "UDEFB %d\n", val); break;
        case __DSB: fprintf (outhdl, "DSB %d\n", val); break;
        case __END: fprintf (outhdl, "END\n", val); break;
        case __CODEORG: fprintf (outhdl, "CODEORG %d\n", val); break;
        case __DATAORG: fprintf (outhdl, "DATAORG %d\n", val); break;
        case __UDEFORG: fprintf (outhdl, "UDEFORG %d\n", val); break; 
        case __SYMBOL:
          fprintf (outhdl, "SYMBOL ");
          foutname (val);
          fprintf (outhdl, "\n");
          break;
        case __CODEDEF:
          p = &name[val*NLAST];
          fprintf (outhdl, "CODEDEF %d,", p[NVALUE]);
          foutname (val);
          fprintf (outhdl, "\n", 0);
          break;
        case __DATADEF:
          p = &name[val*NLAST];
          fprintf (outhdl, "DATADEF %d,", p[NVALUE]);
          foutname (val);
          fprintf (outhdl, "\n", 0);
          break;
        case __UDEFDEF:
          p = &name[val*NLAST];
          fprintf (outhdl, "UDEFDEF %d,", p[NVALUE]);
          foutname (val);
          fprintf (outhdl, "\n", 0);
          break;
	default:
	  fprintf (outhdl, "cmd: %d\n", cmd);
	  break;
      }
    }
  }
}
