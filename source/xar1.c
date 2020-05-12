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
** X-Archiver.  Part 1, I/O Support routines
*/

#define EXTERN
#include "xar.h"


/**********************************************************************/
/*                                                                    */
/*    Compiler startup routines                                       */
/*                                                                    */
/**********************************************************************/

/*
** Initialize all variables 
*/
initialize ()
{
register int i;

  monitor = debug = usercmd = 0;
  objhdl = olbhdl = outhdl = 0;
  objfn[0] = olbfn[0] = outfn[0] = 0;
}

/*
** Process commandline
*/
usage ()
{
  printf ("usage: xar (A|C|D|L|X) <library>[.<ext>] [<object>[.<ext>] [-m]\n");
  printf ("  A  Add a module\n");
  printf ("  C  Create a new library\n");
  printf ("  D  Delete a module\n");
  printf ("  L  List the library\n");
  printf ("  X  Extract a module\n");
  printf ("  -m Monitor\n");
  exit (1);
}

startup (cmdline)
register char *cmdline;
{
register int i, j, ext;

  while (*cmdline) {
    /* Skip spaces */
    while (*cmdline && (*cmdline <= ' '))
      ++cmdline;

    if (*cmdline != '-') {
      if (!usercmd) {
        switch (*cmdline) {
          case 'a': case 'A':
            usercmd = ADDCMD;
            break;
          case 'c': case 'C':
            usercmd = CRECMD;
            break;
          case 'd': case 'D':
            usercmd = DELCMD;
            break;
          case 'l': case 'L':
            usercmd = LISCMD;
            break;
          case 'x': case 'X':
            usercmd = EXTCMD;
            break;
          default:
            usage ();
        }
        ++cmdline;
        if (*cmdline > ' ')
          usage (); /* one letter commands only */
      } else if (!olbfn[0]) {
        /* Copy filename */
        i = ext = 0;
        while (*cmdline && (*cmdline > ' ')) {
          bakfn[i] = olbfn[i] = outfn[i] = *cmdline;
          if (*cmdline == '\\')
            ext = 0;  /* Directory delimiter */
          if (*cmdline == '.')
            ext = i; /* Start of extension */
          ++cmdline;
          ++i;
        }
        /* Now modify extensions */
        if (!ext) {
          olbfn[i+0] = '.';
          olbfn[i+1] = 'O';
          olbfn[i+2] = 'L';
          olbfn[i+3] = 'B';
          olbfn[i+4] = 0;
          ext = i;
        }
        outfn[ext] = '.';
        outfn[ext+1] = '$';
        outfn[ext+2] = '$';
        outfn[ext+3] = '$';
        outfn[ext+4] = 0;
        bakfn[ext] = '.';
        bakfn[ext+1] = 'B';
        bakfn[ext+2] = 'A';
        bakfn[ext+3] = 'K';
        bakfn[ext+4] = 0;
      } else if (!objfn[0]) {
        /* Copy filename */
        i = j = ext = 0;
        while (*cmdline && (*cmdline > ' ')) {
          objfn[i] = modn[j] = *cmdline;
          if ((modn[i] >= 'a') && (modn[i] <= 'z'))
            modn[i] += 'A' - 'a';
          ++j;
          if (*cmdline == '\\')
            j = ext = 0;  /* Directory delimiter */
          if (*cmdline == '.')
            ext = i; /* Start of extension */
          ++cmdline;
          ++i;
        }
        /* Now modify extensions */
        if (!ext) {
          objfn[i+0] = '.';
          objfn[i+1] = 'O';
          objfn[i+2] = 'B';
          objfn[i+3] = 'J';
          objfn[i+4] = 0;
          ext = i;
        }
        modn[j+0] = '.';
        modn[j+1] = 'O';
        modn[j+2] = 'B';
        modn[j+3] = 'J';
        modn[j+4] = 0;
      } else
        usage ();
    } else {
      /* Process option */
      switch (cmdline[1]) {
        case 'm': case 'M':
          monitor = 1;
          break;
        case 'd': case 'D':
          debug = 1;
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
  if (!olbfn[0])
    usage ();
  /* command MUST be supplied */
  if (!usercmd)
    usage ();
  /* commands -a -d needs object name */
  if ((usercmd == ADDCMD) || (usercmd == DELCMD) || (usercmd == EXTCMD)) 
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

/*
** Open the .OLB file, test the header, and load the tables 
*/
open_olb ()
{
register int i, *p;

  olbhdl = mustopen (olbfn, "r");
  if (fread (olbhdr, BPW, HLAST, olbhdl) != HLAST) {
    printf ("error reading .OLB header\n");
    exit (1);
  }
  if (olbhdr[HNAME] > NAMEMAX) {
    printf ("name table too large in .OLB\n");
    exit (1);
  }
  if (olbhdr[HFILE] > FILEMAX) {
    printf ("file table too large in .OLB\n");
    exit (1);
  }
  if (fread (name, BPW, i=olbhdr[HNAME]*NLAST, olbhdl) != i) {
    printf ("error reading .OLB nametable\n");
    exit (1);
  }
  if (olbhdr[HFILE] > 0)
    if (fread (file, BPW, i=olbhdr[HFILE]*FLAST, olbhdl) != i) {
      printf ("error reading .OLB filetable\n");
      exit (1);
    }

  /* duplicate offset fields */
  for (i=0; i<olbhdr[HFILE]; i++) {
    p = &file[i*FLAST];
    p[FOLDOFS] = p[FOFFSET];
  }
}


/**********************************************************************/
/*                                                                    */
/*    Symboltable routines                                            */
/*                                                                    */
/**********************************************************************/

outname (hash)
register int hash;
{
register int i;

  if ((i=name[hash*NLAST+NTAB]) != -1)
    i = outname (i); /* display and get length string */
  else
    i = 0; /* nothing displayed yet */
  printf ("%c", name[hash*NLAST+NCHAR]);
  return i+1; /* Increment length */
}

soutname (hash, str)
register int hash;
register char *str;
{
register int i;

  if ((i=name[hash*NLAST+NTAB]) != -1)
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

  tab = -1;
  len = 0;
  hash = 0;
  while (*ident) {
    start = hash = (hash + *ident * *ident) % olbhdr[HNAME];
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
        if (hash >= olbhdr[HNAME])
          hash -= olbhdr[HNAME];
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
register int i, j, *p;

  printf ("%s\n", VERSION); /* Print banner */
  initialize (); /* initialize all variables */
  startup (cmdline); /* Process commandline options */
  if (debug) {
    printf ("Library  : '%s'\n", olbfn);
    printf ("Object   : '%s'\n", objfn);
  }

  switch (usercmd) {
    case ADDCMD:
      do_add ();
      break;
    case CRECMD:
      do_cre ();
      break;
    case DELCMD:
      do_del ();
      break;
    case LISCMD:
      do_lis ();
      break;
    case EXTCMD:
      do_ext ();
      break;
  }

  if (debug) {
    j=0; for (i=0; i<olbhdr[HNAME]; i++) if (name[i*NLAST+NCHAR]) j++;
    printf ("Names        : %-5d(%-5d)\n", j, olbhdr[HNAME]);
  }
}
