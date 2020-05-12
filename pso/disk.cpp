#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <stat.h>
#include <dos.h>
#include "system.hpp"
#include "disk.hpp"
#include "mmu.hpp"

extern PAGE_TABLE pagetable;

DISK::DISK ()
{
int i;

  /* Initialize handles */
  for (i=0; i<FILEMAX; i++)
    __fhdl[i*LAST+HDL] = 0;
}

int DISK::fopen (char *fname, int mode, int type)
{
register int hdl, ghdl;
register long *fp;
register char *cp;

  /* Locate a free handle */
  for (hdl=6; hdl<FILEMAX; hdl++) {
    fp = &__fhdl[hdl*LAST];
    if (!fp[HDL])
      break;
  }
  if (hdl == FILEMAX) {
    printf ("out of file handles\n");
    exit (1);
  }

  /* Open file */
  switch (mode) {
    case 'W': case 'WS':
      unlink (fname);
      ghdl = creat (fname, S_IREAD|S_IWRITE);
      if (ghdl < 0)
        return ghdl;
      break;
    case 'R': case 'RS':
      ghdl = open (fname, O_RDWR, 0);
      perror ("error opening file ");
      if (ghdl < 0)
        return ghdl;
      break;
    default:
      printf ("file '%s' has unknown open mode '%c'\n", fname, mode);
      exit (1);
  }

  /* Allocate buffers */
  if (!(fp[BUF] = (long)malloc (BUFMAX))) /* MALLOC */
    return -1;

  /* Link it all */
  fp[NAME] = (long)&__name[hdl*40];
  cp = (char*)fp[NAME];
  while (*fname)
    *cp++ = *fname++;
  fp[BUFPOS] = fp[FILPOS] = fp[BUFLEN] = fp[BUFOFS] = 0;
  fp[DIRTY] = 0;
  fp[MODE] = mode;
  fp[HDL] = ghdl;

  return hdl;
}

int DISK::fflush (long *fp)
{
  /* reposition if needed */
  if (fp[BUFPOS] != fp[FILPOS]) {
    if (lseek (fp[HDL], fp[BUFPOS], SEEK_SET) != fp[BUFPOS]) {
      printf ("seek error on file '%s'\n", fp[NAME]);
      exit (1);
    }
  }
  /* writeback */
  if (write (fp[HDL], (void*)fp[BUF], fp[BUFLEN]) != fp[BUFLEN]) {
    printf ("write error on file '%s'\n", fp[NAME]);
    exit (1);
  }
  fp[FILPOS] = fp[BUFPOS] + fp[BUFLEN];
  /* clear bit */
  fp[DIRTY] = 0;
}

int DISK::fload (long *fp)
{
  /* load window */
  if (fp[BUFPOS] != fp[FILPOS]) {
    if (lseek (fp[HDL], fp[BUFPOS], SEEK_SET) != fp[BUFPOS]) {
      printf ("seek error on file '%s'\n", fp[NAME]);
      exit (1);
    }
  }
  if ((fp[BUFLEN] = read (fp[HDL], (void*)fp[BUF], BUFMAX)) < 0) {
    printf ("read error on file '%s'\n", fp[NAME]);
    perror (">>");
    exit (1);
  }
  fp[FILPOS] = fp[BUFPOS] + fp[BUFLEN];
}

int DISK::fread (int fid, ADDRESS addr, int len)
{
register int retlen, ch;
register long *fp;
register char *bufp;

  fp = (long*)&__fhdl[fid*LAST];
  if (!fp[HDL]) {
    printf ("file not opened\n");
    exit (1);
  }
  if (fp[MODE] == 'W') {
    printf ("file '%s' not opened for read\n", fp[NAME]);
    exit (1);
  }

  retlen = 0;
  if (!len)
    len = -1;  /* negative -> ASCII mode */
  bufp = (char*)fp[BUF];
  while (len) {
    /* test if extractpoint in window */
    if (fp[BUFOFS] >= fp[BUFLEN]) {
      /* update dirty buffer */
      if (fp[DIRTY])
        fflush (fp);
      /* init window */
      fp[BUFPOS] += fp[BUFLEN];
      fp[BUFOFS] = 0;
      fload (fp);
      /* test for EOF */
      if (!fp[BUFLEN]) {
        if (len < 0)
          pagetable.write_byte (addr++, 0); /* terminate ASCII buffer */
        return retlen ? retlen : -1 /*EOF*/ ;
      }
    }
    /* extract from window */
    if (len > 0) {
      pagetable.write_byte (addr++, bufp[fp[BUFOFS]++]);
      --len;
      ++retlen;
    } else {
      ch = bufp[fp[BUFOFS]++];
      if (ch == 13)
        ; /* skip <CR> */
      else if (ch == 10) {
        pagetable.write_byte (addr++, 0);
        return retlen;
      } else {
        pagetable.write_byte (addr++, ch);
        ++retlen;
      }
    }
  }
  return retlen;
}

int DISK::fwrite (int fid, ADDRESS addr, int len)
{
register int retlen, ch;
register long *fp;
register char *bufp;

  fp = (long*)&__fhdl[fid*LAST];
  if (!fp[HDL]) {
    printf ("file not opened\n");
    exit (1);
  }
  if (fp[MODE] == 'R') {
    printf ("file '%s' not opened for write\n", fp[NAME]);
    exit (1);
  }

  retlen = 0;
  if (!len)
    len = -1;  /* negative -> ASCII mode */
  bufp = fp[BUF];
  while (len) {
    /* test if insertpoint in window */
    if (fp[BUFOFS] >= BUFMAX) {
      /* update dirty buffer */
      fp[BUFLEN] = BUFMAX;
      if (fp[DIRTY])
        fflush (fp);
      /* init window */
      fp[BUFPOS] += BUFMAX;
      fp[BUFOFS] = fp[BUFLEN] = 0;
      if (fp[MODE] == 'U')
        fload (fp);
    }
    fp[DIRTY] = 1;
    if (len > 0) {
      pagetable.read_byte (addr++, &bufp[fp[BUFOFS]++]);
      --len;
      ++retlen;
    } else {
      pagetable.read_byte (addr++, &ch);
      if (!ch)
        return retlen;
      bufp[fp[BUFOFS]++] = ch;
    }
    if (fp[BUFOFS] > fp[BUFLEN])
      fp[BUFLEN] = fp[BUFOFS];
  }
  return retlen;
}

int DISK::fseek (int fid, int pos)
{
register int i;
register long *fp, len, _pos;
register char *cp;

  _pos = pos;
  _pos &= 0xFFFFL;

  fp = (long*)&__fhdl[fid*LAST];
  if (!fp[HDL]) {
    printf ("file not opened\n");
    exit (1);
  }
  if ((fp[MODE] != 'RS') && (fp[MODE] != 'WS')) {
    printf ("file '%s' not opened for update\n", fp[NAME]);
    exit (1);
  }

  /* test if positioned within current block */
  if ((_pos - fp[BUFPOS] >= 0) && (_pos - fp[BUFPOS] < BUFMAX)) {
    fp[BUFOFS] = _pos-fp[BUFPOS];
    if (fp[BUFOFS] - fp[BUFLEN] >= 0)
      fp[BUFLEN] = fp[BUFOFS] + 1;
    return;
  }
 
  /* first flush buffer if dirty */
  if (fp[DIRTY])
    fflush (fp);

  /* now do a DOS seek */
  while (fp[FILPOS] != _pos) {
    fp[FILPOS] = lseek (_pos, fp[HDL], 0);
    if (fp[FILPOS] != _pos) { 
      /* erase buffer */
      for (i=0,cp=fp[BUF]; i<BUFMAX; i++)
        *cp++ = 0;
      /* go to EOF */
      fp[FILPOS] = lseek (0, fp[HDL], 2);
      /* fill */
      while (fp[FILPOS] - _pos < 0) {
        len = _pos - fp[FILPOS];
        if (len >= BUFMAX)
          len = BUFMAX;
        if (write (fp[HDL], len, fp[BUF]) != len) {
          printf ("write error on file '%s'\n", fp[NAME]);
          exit (1);
        }
        fp[FILPOS] += len; 
      }
      fp[FILPOS] = 0; /* redo loop */
    }
  }

  /* read buffer */
  fp[BUFPOS] = _pos;
  fp[BUFOFS] = fp[BUFLEN] = 0;
  fload (fp);
}

int DISK::fclose (int fid)
{
register long *fp;
register int i;

  fp = (long*)&__fhdl[fid*LAST];
  if (!fp[HDL]) {
    printf ("file not opened\n");
    exit (1);
  }

  if (fp[DIRTY])
    fflush (fp);

  close (fp[HDL]);
  free (fp[BUF]);
  fp[HDL] = 0;
}

int DISK::fdelete (char *fn)
{
  return unlink (fn);
}

int DISK::frename (char *ofn, char *nfn)
{
  return rename (ofn, nfn);
}
