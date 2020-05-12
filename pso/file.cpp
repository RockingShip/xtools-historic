#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <stat.h>
#include <dos.h>
#include <string.h>
#include "file.hpp"

XFILE::XFILE (PAGE_TABLE *pagetable)
{
  XFILE::pagetable = pagetable;
  mode = 0;
  type = 0;
}

int XFILE::fopen (char *fname, int mode, int type)
{
register char *cp;

  /* Open file */
  switch (mode) {
    case 'W': case 'WS':
      unlink (fname);
      hdl = creat (fname, S_IREAD|S_IWRITE);
      if (hdl < 0)
        return hdl;
      break;
    case 'R': case 'RS':
      hdl = open (fname, O_RDWR, 0);
      if (hdl < 0)
        return hdl;
      break;
    default:
      printf ("file '%s' has unknown open mode '%c'\n", fname, mode);
      exit (1);
  }

  /* Link it all */
  strcpy (name, fname);
  bufpos = filpos = buflen = bufofs = 0;
  dirty = 0;
  XFILE::mode = mode;

  return 0;
}

int XFILE::fflush ()
{
  /* reposition if needed */
  if (bufpos != filpos) {
    if (lseek (hdl, bufpos, SEEK_SET) != bufpos) {
      printf ("seek error on file '%s'\n", name);
      exit (1);
    }
  }
  /* writeback */
  if (write (hdl, buf, buflen) != buflen) {
    printf ("write error on file '%s'\n", name);
    exit (1);
  }
  filpos = bufpos + buflen;
  /* clear bit */
  dirty = 0;
}

int XFILE::fload ()
{
  /* load window */
  if (bufpos != filpos) {
    if (lseek (hdl, bufpos, SEEK_SET) != bufpos) {
      printf ("seek error on file '%s'\n", name);
      exit (1);
    }
  }
  if ((buflen = read (hdl, buf, BUFMAX)) < 0) {
    printf ("read error on file '%s'\n", name);
    perror (">>");
    exit (1);
  }
  filpos = bufpos + buflen;
}

int XFILE::fread (ADDRESS addr, int len)
{
register int retlen, ch;

  if (!mode)
    return -1;

  if (mode == 'W') {
    printf ("file '%s' not opened for read\n", name);
    exit (1);
  }

  retlen = 0;
  if (!len)
    len = -1;  /* negative -> ASCII mode */
  while (len) {
    /* test if extractpoint in window */
    if (bufofs >= buflen) {
      /* update dirty buffer */
      if (dirty)
        fflush ();
      /* init window */
      bufpos += buflen;
      bufofs = 0;
      fload ();
      /* test for EOF */
      if (!buflen) {
        if (len < 0)
          pagetable->write_byte (addr++, 0); /* terminate ASCII buffer */
        return retlen ? retlen : -1 /*EOF*/ ;
      }
    }
    /* extract from window */
    if (len > 0) {
      pagetable->write_byte (addr++, buf[bufofs++]);
      --len;
      ++retlen;
    } else {
      ch = buf[bufofs++];
      if (ch == 13)
        ; /* skip <CR> */
      else if (ch == 10) {
        pagetable->write_byte (addr++, 0);
        return retlen;
      } else {
        pagetable->write_byte (addr++, ch);
        ++retlen;
      }
    }
  }
  return retlen;
}

int XFILE::fwrite (ADDRESS addr, int len)
{
register int retlen;
char ch;

  if (!mode)
    return -1;

  if (mode == 'R') {
    printf ("file '%s' not opened for write\n", name);
    exit (1);
  }

  retlen = 0;
  if (!len)
    len = -1;  /* negative -> ASCII mode */
  while (len) {
    /* test if insertpoint in window */
    if (bufofs >= BUFMAX) {
      /* update dirty buffer */
      buflen = BUFMAX;
      if (dirty)
        fflush ();
      /* init window */
      bufpos += BUFMAX;
      bufofs = buflen = 0;
      if (mode == 'U')
        fload ();
    }
    dirty = 1;
    if (len > 0) {
      pagetable->read_byte (addr++, &buf[bufofs++]);
      --len;
      ++retlen;
    } else {
      pagetable->read_byte (addr++, &ch);
      if (!ch)
        return retlen;
      buf[bufofs++] = ch;
      ++retlen;
    }
    if (bufofs > buflen)
      buflen = bufofs;
  }
  return retlen;
}

int XFILE::fseek (int pos)
{
register int i;
register long len, _pos;
register char *cp;

  if (!mode)
    return -1;

  _pos = pos;
  _pos &= 0xFFFFL;

  if ((mode != 'RS') && (mode != 'WS')) {
    printf ("file '%s' not opened for update\n", name);
    exit (1);
  }

  /* test if positioned within current block */
  if ((_pos - bufpos >= 0) && (_pos - bufpos < BUFMAX)) {
    bufofs = _pos-bufpos;
    if (bufofs - buflen >= 0)
      buflen = bufofs + 1;
    return;
  }
 
  /* first flush buffer if dirty */
  if (dirty)
    fflush ();

  /* now do a DOS seek */
  while (filpos != _pos) {
    filpos = lseek (_pos, hdl, 0);
    if (filpos != _pos) {
      /* erase buffer */
      for (i=0,cp=buf; i<BUFMAX; i++)
        *cp++ = 0;
      /* go to EOF */
      filpos = lseek (0, hdl, 2);
      /* fill */
      while (filpos - _pos < 0) {
        len = _pos - filpos;
        if (len >= BUFMAX)
          len = BUFMAX;
        if (write (hdl, buf, len) != len) {
          printf ("write error on file '%s'\n", name);
          exit (1);
        }
        filpos += len;
      }
      filpos = 0; /* redo loop */
    }
  }

  /* read buffer */
  bufpos = _pos;
  bufofs = buflen = 0;
  fload ();
}

int XFILE::fclose ()
{
register int i;

  if (!mode)
    return -1;

  if (dirty)
    fflush ();

  close (hdl);
  hdl = 0;

  return 0;
}
