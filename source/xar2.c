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
** X-Archiver.  Part 2, Actions
*/

#define EXTERN extern
#include "xar.h"

objmap ()
{
register int i, *p, len;

  printf ("id       filename          offset       length   \n");
  printf ("-- -------------------- ------------ ------------\n");

  for (i=0; i<olbhdr[HFILE]; i++) {
    p = &file[i*FLAST];
    printf ("%2d ", i+1);
    len = outname (p[FNAME]);
    while (len++ <= 20)
      printf (" ");
    printf ("%04x (%5d) %04x (%5d)\n",
            p[FOFFSET], p[FOFFSET], p[FLENGTH], p[FLENGTH]);
  }
}

symmap (start)
register int start;
{
register int ch, *p, hash, tab;

  tab = (!start) ? 0 : start;
  for (ch='!'; ch<='~'; ch++) {
    hash = (start + ch * ch) % olbhdr[HNAME];
    while (1) {
      p = &name[hash*NLAST];
      if ((p[NCHAR] == ch) && (p[NTAB] == tab)) {
        if (p[NLIB] != -1) {
          printf ("%2d ", p[NLIB]+1);
          outname (hash); 
          printf ("\n");
        }
        symmap (hash);
        break; /* Inner loop */
      } else if (!p[NCHAR]) {
        break; /* Inner loop */
      } else {
        hash += ch;
        if (hash >= olbhdr[HNAME])
          hash -= olbhdr[HNAME];
      }
    }
  }
}

read_byte()
{
char arr[1];

  if (fread (arr, 1, 1, objhdl) != 1) {
    printf("missing .END (use -v to discover where)\n");
    exit(1);
  }

  /* return unsigned */
  return arr[0] & 0xff;
}

read_word(hdl)
int hdl;
{
char arr[2];
int w;

  if (fread (arr, 1, 2, hdl) != 2) {
    printf("missing .END (use -v to discover where)\n");
    exit(1);
  }

  /* return signed */
  w = arr[0] << 8 | (arr[1] & 0xff);
  w |= -(w & (1 << SBIT));
  return w;
}

write_byte(byte)
char byte;
{
  fwrite (&byte, 1, 1, outhdl);
}

write_word(word)
int word;
{
  char arr[2];

  arr[0] = word>>8;
  arr[1] = word;

  fwrite (arr, 1, 2, outhdl);
}

do_lis ()
{
  open_olb ();
  printf ("Object statistics : \n");
  objmap ();
  printf ("\nSymboltable : \n\n");
  symmap (0);
  fclose (olbhdl);
}

do_cre ()
{
register int i;

  /* initialize header */
  olbhdr[HNAME] = NAMEMAX;
  olbhdr[HFILE] = 0;
  /* Initialize nametable */
  for (i=0; i<NAMEMAX; i++) 
    name[i*NLAST+NLIB] = -1;

  /* open outputfile */
  unlink (outfn);
  outhdl = mustopen (outfn, "w");

  /* Writeout */
  for (i=0; i<HLAST; i++)
    write_word(olbhdr[i]);
  for (i=0; i<olbhdr[HNAME]*NLAST; i++)
    write_word(name[i]);

  /* close and rename */
  fclose (outhdl);
  unlink (bakfn);
  rename (olbfn, bakfn);
  rename (outfn, olbfn);
}

do_add ()
{
char cmd;
register int objlen, olblen, *p, i;
int error, hash, objinx;

  /* open inputfile */
  open_olb ();

  if (verbose)
    printf("Loading module %s\n", objfn);

  /* first delete any existing occurrences */
  dohash (modn, &hash);
  for (objinx=0; objinx<olbhdr[HFILE]; objinx++)
    if (file[objinx*FLAST+FNAME] == hash)
      break;
  if (objinx<olbhdr[HFILE]) {
    /* found existing entry, overwrite */
    for (i=0; i<olbhdr[HNAME]; i++) {
      p = &name[i*NLAST];
      if (p[NLIB] == objinx)
        p[NLIB] = -1;
    }
    /* update file entry */
    p = &file[objinx*FLAST];
    p[FOLDOFS] = 0;
  } else {
    objinx = olbhdr[HFILE]++;
    /* create new object at end of list */
    if (objinx >= FILEMAX) {
      printf ("Too many modules in library\n");
      exit (1);
    }
    /* update file entry */
    p = &file[objinx*FLAST];
    dohash (modn, &p[FNAME]);
    p[FOLDOFS] = 0;
  }

  /* open objectfile */
  objhdl = mustopen (objfn, "r");

  /* read object, calc length and insert all found symbols */
  error = 0;
  objlen = 0;
  cmd = -1;
  while (cmd != __END) {
    cmd = read_byte();
    if (cmd < 0) {
      datlen = -cmd;
      fread (datbuf, 1, datlen, objhdl);
      objlen += datlen + 1;
    } else {
      switch (cmd) {
        case __ADD: case __SUB: case __MUL: case __DIV: case __MOD:
        case __LSR: case __LSL: case __XOR: case __AND: case __OR:
        case __NOT: case __NEG: case __SWAP:
        case __POPB: case __POPW:
          objlen += 1;
          break;
        case __PUSHB: case __CODEB: case __DATAB: case __UDEFB:
          read_byte();
          objlen += 2;
          break;
        case __PUSHW: case __CODEW: case __DATAW: case __UDEFW:
          read_word(objhdl);
          objlen += BPW + 1;
          break;
        case __SYMBOL:
          /* Push symbol value on stack */
          datlen = read_byte(); /* length */
          fread (datbuf, 1, datlen, objhdl); /* symbol */
          objlen += 2 + datlen;
          break;
        case __DSB:
	  /* skip specified number of bytes in current segment */
          read_word(objhdl); /* skipcount */
          objlen += 1 + BPW;
          break;
        case __END:
           objlen += 1;
           break;
        case __CODEDEF: case __DATADEF: case __UDEFDEF:
          /* symbol definition */
          read_word(objhdl); /* symbol offset */
          datlen = read_byte(); /* length */
          fread (datbuf, 1, datlen, objhdl); /* symbol */
          objlen += 1 + BPW + 1 + datlen;

          /* locate symbol in table and insert */
          datbuf[datlen] = 0;
          dohash (datbuf, &hash);
          p = &name[hash*NLAST];
          if (p[NLIB] != -1) {
            /* get name of library already containing symbol */
            printf ("Symbol '%s' already defined in module ", datbuf);
            soutname (file[p[NLIB]*FLAST+FNAME], datbuf);
            printf ("%s\n", datbuf);
            ++error;
          } else {
            /* new symbol */
            p[NLIB] = objinx;
            if (debug)
              printf ("SYMDEF: %s\n", datbuf);
          }
          break;
        case __CODEORG: case __DATAORG: case __UDEFORG:
          read_word(objhdl); /* segment offset */
          objlen += 1 + BPW;
          break;
        default:
          printf ("unknown command %d\n", cmd);
          exit (1); 
          break;
      }
    }
  }

  /* save length of module */
  file[objinx*FLAST+FLENGTH] = objlen;

  /* generate new offsets for existing modules */
  olblen = (HLAST*BPW) +
           (olbhdr[HNAME]*NLAST*BPW) + 
           (olbhdr[HFILE]*FLAST*BPW) ;
  for (i=0; i<olbhdr[HFILE]; i++) {
    p = &file[i*FLAST];
    p[FOFFSET] = olblen;
    olblen += p[FLENGTH];
  }

  /* if no errors occurred then generate new library */
  if (error) {
    printf ("module not inserted\n");
    exit (1);
  }
  if (debug)
    printf ("module length: %d\n", objlen);

  /* build new library */
  unlink (outfn);
  outhdl = mustopen (outfn, "w");

  /* cleanup header */
  for (i=0; i<olbhdr[HFILE]; i++)
    file[i*FLAST+FOLDOFS] = 0;

  /* Writeout */
  for (i=0; i<HLAST; i++)
    write_word(olbhdr[i]);
  for (i=0; i<olbhdr[HNAME]*NLAST; i++)
    write_word(name[i]);
  for (i=0; i<olbhdr[HFILE]*FLAST; i++)
    write_word(file[i]);
 
  /* copy objects and append new object */
  for (i=0; i<olbhdr[HFILE]; i++) {
    p = &file[i*FLAST];
    if (p[FOLDOFS])
      copy_obj (olbhdl, p[FOLDOFS], p[FLENGTH]);
    else
      copy_obj (objhdl, 0, p[FLENGTH]);
  }

  /* close and rename */
  fclose (outhdl);
  unlink (bakfn);
  rename (olbfn, bakfn);
  rename (outfn, olbfn);
}

do_del ()
{
char cmd;
register int olblen, *p, i;
int error, hash, objinx;

  /* open inputfile */
  open_olb ();

  /* locate module in filetable */
  dohash (modn, &hash);
  for (objinx=0; objinx<olbhdr[HFILE]; objinx++)
    if (file[objinx*FLAST+FNAME] == hash)
      break;
  if (objinx>=olbhdr[HFILE]) {
    printf ("module not found\n");
    exit (1);
  }

  /* remove all symbol references */
  for (i=0; i<olbhdr[HNAME]; i++) {
    p = &name[i*NLAST];
    if (p[NLIB] == objinx)
      p[NLIB] = -1;
    else if (p[NLIB] > objinx)
      --p[NLIB];
  }

  /* remove fileentry */
  for (i=objinx; i<olbhdr[HFILE]; i++) {
    p = &file[i*FLAST];
    p[FNAME]   = p[FNAME+FLAST];
    p[FOLDOFS] = p[FOLDOFS+FLAST];
    p[FLENGTH] = p[FLENGTH+FLAST];
  }
  --olbhdr[HFILE];

  /* generate new offsets for remaining modules */
  olblen = (HLAST*BPW) +
           (olbhdr[HNAME]*NLAST*BPW) + 
           (olbhdr[HFILE]*FLAST*BPW) ;
  for (i=0; i<olbhdr[HFILE]; i++) {
    p = &file[i*FLAST];
    p[FOFFSET] = olblen;
    olblen += p[FLENGTH];
  }

  /* build new library */
  unlink (outfn);
  outhdl = mustopen (outfn, "w");

  /* cleanup header */
  for (i=0; i<olbhdr[HFILE]; i++)
    file[i*FLAST+FOLDOFS] = 0;

	/* Writeout */
  for (i=0; i<HLAST; i++)
    write_word(olbhdr[i]);
  for (i=0; i<olbhdr[HNAME]*NLAST; i++)
    write_word(name[i]);
  for (i=0; i<olbhdr[HFILE]*FLAST; i++)
    write_word(file[i]);
 
  /* copy objects and append new object */
  for (i=0; i<olbhdr[HFILE]; i++) {
    p = &file[i*FLAST];
    copy_obj (olbhdl, p[FOLDOFS], p[FLENGTH]);
  }

  /* close and rename */
  fclose (outhdl);
  unlink (bakfn);
  rename (olbfn, bakfn);
  rename (outfn, olbfn);
}

do_ext ()
{
char cmd;
register int olblen, *p, i;
int error, hash, objinx;

  /* open inputfile */
  open_olb ();

  /* locate module in filetable */
  dohash (modn, &hash);
  for (objinx=0; objinx<olbhdr[HFILE]; objinx++) {
    p = &file[objinx*FLAST];
    if (p[FNAME] == hash)
      break;
  }
  if (objinx>=olbhdr[HFILE]) {
    printf ("module not found\n");
    exit (1);
  }

  /* open object file as outhdl (used by copy_obj) */
  outhdl = mustopen (objfn, "w");
  copy_obj (olbhdl, p[FOFFSET], p[FLENGTH]);

  fclose (outhdl);
  fclose (olbhdl);
}

copy_obj (hdl, ofs, len)
register int hdl, ofs, len;
{
register int i, tmplen;

  fseek (hdl, ofs, 0);
  while (len > 0) {
    tmplen = len;
    if (tmplen > 512)
      tmplen = 512;
    if ((i=fread (datbuf, 1, tmplen, hdl)) != tmplen) {
      printf ("error reading .OLB during copy\n");
      exit (1);
    }
    if ((i=fwrite (datbuf, 1, tmplen, outhdl)) != tmplen) {
      printf ("error writinging .OLB during copy\n");
      exit (1);
    }
    len -= tmplen;
  }
}
