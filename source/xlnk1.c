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
** X-Linker.  Part 1, I/O Support
*/

#define EXTERN
#include "xlnk.h"


/**********************************************************************/
/*                                                                    */
/*    Compiler startup routines                                       */
/*                                                                    */
/**********************************************************************/

add_res (sym, typ)
char *sym;
int typ;
{
int hash;
register int *p;

  dohash (sym, &hash);
  p = &name[hash*NLAST];
  p[NTYPE] = typ;
  p[NMODULE] = -1;
  p[NVALUE] = 0;
}

initialize ()
{
register int i,*p;
int hash;

  undef = monitor = debug = maklis = 0;
  stksiz = 2000;
  outhdl = lishdl = inphdl = 0;
  inpfn[0] = outfn[0] = 0;
  datlen = 0;
  file1inx = file2inx = 0;

#ifdef DYNAMIC
  ! check this first
  name = calloc (NAMEMAX*BPW*NLAST);
  file = calloc (FILEMAX*BPW*FLAST);
#endif;

  /* reset tables */
  for (i=0; i<NAMEMAX; i++) {
    p = &name[i*NLAST];
    p[NCHAR] = p[NTYPE] = p[NVALUE] = 0;
  }

  /* reserve first entry so it terminates lists */
  name[0*NLAST+NCHAR] = '?';

  /* reset positions */
  pass = 1;
 
  /* predefined symbols */
  add_res ("___START",    UNDEF);
  add_res ("___STACKLEN", ABS);
  add_res ("___CODEBASE", ABS);
  add_res ("___CODELEN"), ABS;
  add_res ("___DATABASE", ABS);
  add_res ("___DATALEN",  ABS);
  add_res ("___UDEFBASE", ABS);
  add_res ("___UDEFLEN",  ABS);
}

/*
** Process commandline
*/
usage ()
{
  printf ("usage: xlnk <file>[.<ext>] {,<file>[.<ext>]} {-x <file>[.<ext>]}\n");
  printf ("            [-o <file>[.<ext>]] [-s<stksiz>] [-u] [-m] [-d] [-l]\n");
  exit (1);
}

startup (cmdline)
register char *cmdline;
{
register int i, ext, *p;
int hash;

  while (*cmdline) {
    /* Skip spaces */
    while (*cmdline && (*cmdline <= ' '))
      ++cmdline;

    if (*cmdline != '-') {
      /* Copy filename */
      i = ext = 0;
      while (*cmdline && (*cmdline > ' ')) {
        inpfn[i] = *cmdline;
        if (!file1inx) 
          outfn[i] = lisfn[i] = *cmdline;
        if (*cmdline == '\\')
          ext = 0;  /* Directory delimiter */
        if (*cmdline == '.')
          ext = i; /* Start of extension */
        ++cmdline;
        ++i;
      }
      /* Now modify extensions */
      if (!ext) {
        inpfn[i+0] = '.';
        inpfn[i+1] = 'O';
        inpfn[i+2] = 'B';
        inpfn[i+3] = 'J';
        inpfn[i+4] = 0;
        ext = i;
      }
      if (!file1inx) {
        outfn[ext] = lisfn[ext] = '.';
        outfn[ext+1] = 'I';
        outfn[ext+2] = 'M';
        outfn[ext+3] = 'G';
        outfn[ext+4] = 0;
        lisfn[ext+1] = 'M';
        lisfn[ext+2] = 'A';
        lisfn[ext+3] = 'P';
        lisfn[ext+4] = 0;
      }
      /* Insert object into filetable */
      if (file1inx >= FILEMAX)
        fatal ("too many files");
      p = &file1[file1inx++*FLAST];
      dohash (inpfn, &hash);
      p[FFILE] = hash;
      p[FLIB] = -1;
      p[FCODEBASE] = p[FCODELEN] = p[FCODEPOS] = 0;
      p[FDATABASE] = p[FUDEFLEN] = p[FDATAPOS] = 0;
      p[FUDEFBASE] = p[FDATALEN] = p[FUDEFPOS] = 0;
    } else {
      /* Process option */
      switch (cmdline[1]) {
        case 'x': case 'X':
          /* skip -x */
          cmdline += 2; 
          /* Skip spaces */
          while (*cmdline && (*cmdline <= ' '))
            ++cmdline;

          /* Copy filename */
          i = ext = 0;
          while (*cmdline && (*cmdline > ' ')) {
            inpfn[i] = *cmdline;
            if (*cmdline == '\\')
              ext = 0;  /* Directory delimiter */
            if (*cmdline == '.')
              ext = i; /* Start of extension */
            ++cmdline;
            ++i;
          }
          /* Now modify extensions */
          if (!ext) {
            inpfn[i+0] = '.';
            inpfn[i+1] = 'O';
            inpfn[i+2] = 'L';
            inpfn[i+3] = 'B';
            inpfn[i+4] = 0;
            ext = i;
          }
          /* Insert file into filetable */
          if (file1inx >= FILEMAX)
            fatal ("too many files");
          p = &file1[file1inx++*FLAST];
          dohash (inpfn, &hash);
          p[FFILE] = -1;
          p[FLIB] = hash;
          break;
        case 'o': case 'O':
          /* skip -o */
          cmdline += 2; 
          /* Skip spaces */
          while (*cmdline && (*cmdline <= ' '))
            ++cmdline;

          /* Copy filename */
          i = ext = 0;
          while (*cmdline && (*cmdline > ' ')) {
            lisfn[i] = outfn[i] = *cmdline;
            if (*cmdline == '\\')
              ext = 0;  /* Directory delimiter */
            if (*cmdline == '.')
              ext = i; /* Start of extension */
            ++cmdline;
            ++i;
          }
          /* Now modify extensions */
          if (!ext) {
            outfn[i+0] = '.';
            outfn[i+1] = 'I';
            outfn[i+2] = 'M';
            outfn[i+3] = 'G';
            outfn[i+4] = 0;
            lisfn[i+0] = '.';
            lisfn[i+1] = 'M';
            lisfn[i+2] = 'A';
            lisfn[i+3] = 'P';
            lisfn[i+4] = 0;
            ext = i;
          }
          break;
        case 's': case 'S':
          /* skip -s */
          cmdline += 2; 
          /* Skip spaces */
          while (*cmdline && (*cmdline <= ' '))
            ++cmdline;
          /* load value */
          stksiz = 0;
          while ((*cmdline >= '0') && (*cmdline <= '9'))
            stksiz = stksiz * 10 + *cmdline++ - '0';
          break;
        case 'u': case 'U':
          undef = 1;
          break;
        case 'm': case 'M':
          monitor = 1;
          break;
        case 'd': case 'D':
          debug = 1;
          break;
        case 'l': case 'L':
          maklis = 1;
          break;
        default:
          usage ();
          break;
      }
      /* Skip switch */
      while (*cmdline && (*cmdline > ' '))
        ++cmdline;
    }
  }

  /* filename MUST be supplied */
  if (!outfn[0])
    usage ();
}

/*
** Open all files
*/
mustopen(fn, mode)
char *fn;
char *mode;
{
int fd;

fd=fopen(fn, mode);
if (fd > 0)
    return fd;
printf (perror("fopen(%s,%s) returned", fn, mode));
  exit (1);
}

openfile ()
{
register int i, *p;
  
  outhdl = mustopen (outfn, "w");
  if (maklis)
    lishdl = mustopen (lisfn, "w");
}

/*
** Open the .OLB file, test the header, and load the tables 
*/
open_olb ()
{
register int i, *p;

  inphdl = mustopen (inpfn, "r");
  if (fread (lbhdr, BPW, LBHLAST, inphdl) != LBHLAST) {
    printf ("error reading .OLB header\n");
    exit (1);
  }
  if (lbhdr[LBHNAME] > NAMEMAX) {
    printf ("name table too large in .OLB\n");
    exit (1);
  }
  if (lbhdr[LBHFILE] > FILEMAX) {
    printf ("file table too large in .OLB\n");
    exit (1);
  }
  if (fread (lbname, BPW, i=lbhdr[LBHNAME]*LBNLAST, inphdl) != i) {
    printf ("error reading .OLB nametable\n");
    exit (1);
  }
  if (lbhdr[LBHFILE] > 0)
    if (fread (lbfile, BPW, i=lbhdr[LBHFILE]*LBFLAST, inphdl) != i) {
      printf ("error reading .OLB filetable\n");
      exit (1);
    }
}


/**********************************************************************/
/*                                                                    */
/*    Input support routines                                          */
/*                                                                    */
/**********************************************************************/


/**********************************************************************/
/*                                                                    */
/*    Symboltable routines                                            */
/*                                                                    */
/**********************************************************************/

lbsoutname (hash, str)
register int hash;
register char *str;
{
register int i;

  if ((i=lbname[hash*LBNLAST+LBNTAB]) != -1)
    str = lbsoutname (i, str);
  *str++ = lbname[hash*LBNLAST+LBNCHAR];
  *str = 0;
  return str;
}

/*
** Get the (unique) hashed value for symbol, return length
*/
lbdohash (ident, retval)
register char *ident;
int *retval;
{
register int start, hash, tab, len, *p;

  tab = -1;
  len = 0;
  hash = 0;
  while (*ident) {
    start = hash = (hash + *ident * *ident) % lbhdr[LBHNAME];
    while (1) {
      p = &lbname[hash*LBNLAST];
      if ((p[LBNCHAR] == *ident) && (p[LBNTAB] == tab)) {
        tab = hash;
        break; /* Inner loop */
      } else if (!p[LBNCHAR]) {
        p[LBNCHAR] = *ident;
        p[LBNTAB] = tab;
        tab = hash;
        break; /* Inner loop */
      } else {
        hash += *ident;
        if (hash >= lbhdr[LBHNAME])
          hash -= lbhdr[LBHNAME];
        if (hash == start) {
          printf ("name table overflow\n");
          exit (1);
        }
      }
    }
    ++ident;
    ++len;
  }
  *retval = tab;
  return len;
}

outname (hash)
register int hash;
{
register int i;

  i=name[hash*NLAST+NTAB];
  if (i)
    i = outname (i); /* display and get length string */
  else
    i = 0; /* nothing displayed yet */
  printf ("%c", name[hash*NLAST+NCHAR]);
  return i+1; /* Increment length */
}

foutname (hash)
register int hash;
{
register int i;

  i=name[hash*NLAST+NTAB];
  if (i)
    i = foutname (i); /* display and get length string */
  else
    i = 0; /* nothing displayed yet */
  fprintf (lishdl, "%c", name[hash*NLAST+NCHAR]);
  return i+1; /* Increment length */
}

soutname (hash, str)
register int hash;
register char *str;
{
register int i;

  i=name[hash*NLAST+NTAB];
  if (i)
    str = soutname (i, str);
  *str++ = name[hash*NLAST+NCHAR];
  *str = 0;
  return str;
}

/*
** Get the (unique) hashed value for symbol, return length
*/
dohash (ident, retval)
register char *ident;
int *retval;
{
register int start, hash, tab, len, *p;

  tab = 0;
  len = 0;
  hash = 0;
  while (*ident) {
    start = hash = (hash + *ident * *ident) % NAMEMAX;
    while (1) {
      p = &name[hash*NLAST];
      if ((p[NCHAR] == *ident) && (p[NTAB] == tab)) {
        tab = hash;
        break; /* Inner loop */
      } else if (!p[NCHAR]) {
        p[NCHAR] = *ident;
        p[NTAB] = tab;
        tab = hash;
        break; /* Inner loop */
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


/**********************************************************************/
/*                                                                    */
/*    Error routines                                                  */
/*                                                                    */
/**********************************************************************/

fatal (msg)
char *msg;
{
  printf ("%s\n", msg);
  exit (1);
}


/**********************************************************************/
/*                                                                    */
/*    Main                                                            */
/*                                                                    */
/**********************************************************************/

/*
** Execution starts here
*/
main (cmdline)
char *cmdline;
{
register int i, j, *p, len;
int hash;

  printf ("%s\n", VERSION); /* Print banner */
  initialize (); /* initialize all variables */
  
  startup (cmdline); /* Process commandline options */
  openfile ();       /* Open all files */
  process ();        /* start linking */

  if (maklis) {
    fprintf (lishdl, "Object statistics : \n");
    objmap ();
    fprintf (lishdl, "\nSymboltable : \n\n");
    symmap (0);
  }

  if (debug) {
    j=0; for (i=0; i<NAMEMAX; i++) if (name[i*NLAST+NCHAR]) j++;
    printf ("Names        : %5d/%5d)\n", j, NAMEMAX);
  }

  return 0;
}

objmap ()
{
register int i, *p, len;

  fprintf (lishdl, "                                                CODE       DATA       UDEF   \n");
  fprintf (lishdl, "id         module             library         BASE LEN   BASE LEN   BASE LEN \n");
  fprintf (lishdl, "-- -------------------- --------------------  ---- ----  ---- ----  ---- ----\n");

  for (i=0; i<file2inx; i++) {
    p = &file2[i*FLAST];
    fprintf (lishdl, "%2d ", i+1);
    len = foutname (p[FFILE]);
    while (len++ <= 20)
      fprintf (lishdl, " ");
    if (p[FLIB] == -1)
      fprintf (lishdl, "                     ");
    else {
      len = foutname (p[FLIB]);
      while (len++ <= 20)
        fprintf (lishdl, " ");
    }
    fprintf (lishdl, " %04x %04x  %04x %04x  %04x %04x\n",
                     p[FCODEBASE], p[FCODELEN], 
                     p[FDATABASE], p[FDATALEN], 
                     p[FUDEFBASE], p[FUDEFLEN]);
  }
}

symmap (start)
register int start;
{
register int ch, *p, hash, tab;

  tab = (!start) ? 0 : start;
  for (ch='!'; ch<='~'; ch++) {
    hash = (start + ch * ch) % NAMEMAX;
    while (1) {
      p = &name[hash*NLAST];
      if ((p[NCHAR] == ch) && (p[NTAB] == tab)) {
        if (p[NTYPE]) {
          fprintf (lishdl, "%2d %04x ", p[NMODULE]+1, p[NVALUE]);
          if (p[NTYPE] == UNDEF)
            fprintf (lishdl, "**** ");
          else if (p[NTYPE] == ABS)
            fprintf (lishdl, "ABS  ");
          else if (p[NTYPE] == CODE)
            fprintf (lishdl, "CODE ");
          else if (p[NTYPE] == DATA)
            fprintf (lishdl, "DATA ");
          else if (p[NTYPE] == UDEF)
            fprintf (lishdl, "UDEF ");
          else
            fprintf (lishdl, "????");
          foutname (hash); 
          fprintf (lishdl, "\n");
        }
        symmap (hash);
        break; /* Inner loop */
      } else if (!p[NCHAR]) {
        break; /* Inner loop */
      } else {
        hash += ch;
        if (hash >= NAMEMAX)
          hash -= NAMEMAX;
      }
    }
  }
}
