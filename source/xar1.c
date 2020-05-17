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
register int i, *p;

  verbose = debug = usercmd = 0;
  objhdl = olbhdl = outhdl = 0;

  objfn[0] = olbfn[0] = outfn[0] = 0;

  /* reset tables */
  for (i=0; i<NAMEMAX; i++) {
    p = &name[i*NLAST];
    p[NCHAR] = p[NLIB] = 0;
  }

  /* reserve first entry so it terminates lists */
  name[0*NLAST+NCHAR] = '?';
}

/*
** Process commandline
*/
usage ()
{
  printf ("X-Archiver, Version %s\n\n", getversion());

  printf ("usage: xar (a|c|d|l|x) <library>[.<ext>] [<object>[.<ext>] ... \n");
  printf ("  a  Add a module\n");
  printf ("  c  Create a new library\n");
  printf ("  d  Delete a module\n");
  printf ("  t  List the library\n");
  printf ("  x  Extract a module\n");
  printf ("  -v Verbose\n");
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
  argv++; /* skip argv[0] */
  while (*argv) {
    register char *arg;
    arg = *argv++;


    if (*arg != '-') {
      if (!usercmd) {
        switch (*arg) {
          case 'a':
            usercmd = ADDCMD;
            break;
          case 'c':
            usercmd = CRECMD;
            break;
          case 'd':
            usercmd = DELCMD;
            break;
          case 't':
            usercmd = LISCMD;
            break;
          case 'x':
            usercmd = EXTCMD;
            break;
          default:
            usage ();
        }
        if (*++arg)
          usage (); /* one letter commands only */
      } else if (!olbfn[0]) {
	fext(olbfn, arg, ".xa", 0);
        fext(bakfn, arg, ".bak", 1);
        fext(outfn, arg, ".tmp", 1);
      } else if (!objfn[0]) {
        fext(objfn, arg, ".xo", 0);
        fext(modn, arg, "", 1);
      } else
        usage ();
    } else {
      /* Process option */
      arg++;
      switch (*arg++) {
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

  /* filename MUST be supplied */
  if (!outfn[0])
    usage ();
  /* command MUST be supplied */
  if (!usercmd)
    usage ();
  /* commands add/del/extract needs object name */
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
  printf ("fopen(%s,%s) failed\n", fn, mode);
  exit (1);
}

/*
** Open the .OLB file, test the header, and load the tables 
*/
open_olb ()
{
register int i, *p;

  /* open, allow failure */
  olbhdl = fopen (olbfn, "r");

  if (!olbhdl) {
    if (verbose)
      printf("Creating library %s\n", olbfn);

    /* init header for empty archive */
    olbhdr[HNAME] = NAMEMAX;
    /* set all symbols to 'not in library' */
    for (i=0; i<NAMEMAX; i++)
      name[i*NLAST+NLIB] = -1;
    return;
  }

  if (verbose)
    printf("Loading library %s %d\n", olbfn, olbhdl);

  for (i=0; i<HLAST; i++)
    olbhdr[i] = read_word_olb();
  if (olbhdr[HNAME] > NAMEMAX)
    fatal ("name table too large in .OLB\n");
  if (olbhdr[HFILE] > FILEMAX)
    fatal ("file table too large in .OLB\n");
  for (i=0; i<olbhdr[HNAME]*NLAST; i++)
    name[i] = read_word_olb();
  if (olbhdr[HFILE] > 0)
    for (i=0; i<olbhdr[HFILE]*FLAST; i++)
      file[i] = read_word_olb();

  /* duplicate offset fields */
  for (i=0; i<olbhdr[HFILE]; i++) {
    p = &file[i*FLAST];
    p[FOLDOFS] = p[FOFFSET];
  }
}

read_word_olb()
{
char arr[2];
int w;

  if (fread (arr, 1, 2, olbhdl) != 2) {
    printf("missing .END (use -v to discover where)\n");
    exit(1);
  }

  /* return signed */
  w = arr[0] << 8 | (arr[1] & 0xff);
  w |= -(w & (1 << SBIT));
  return w;
}

error(msg)
char *msg;
{
  errflag = 1;
  printf ("%s");
}

fatal (msg)
char *msg;
{
error (msg);
exit (0);
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

  i=name[hash*NLAST+NTAB];
  if (i)
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
main (argc, argv)
int argc;
int *argv;
{
register int i, j, *p;

  initialize (); /* initialize all variables */
  startup (argv); /* Process commandline options */
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
    printf ("Names        : %5d/%5d\n", j, olbhdr[HNAME]);
  }

  return 0;
}
