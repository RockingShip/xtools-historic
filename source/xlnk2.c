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
** X-Linker.  Part 2, Link control
*/

#define EXTERN extern
#include "xlnk.h"

objerr (fp, msg, curseg)
register int *fp;
char *msg;
int curseg;
{
  printf (msg);
  printf (" at ");
  if (curseg == CODESEG)
    printf ("C%-04x", fp[FCODEPOS]);
  else if (curseg == DATASEG)
    printf ("D%-04x", fp[FDATAPOS]);
  else
    printf ("U%-04x", fp[FUDEFPOS]);
  printf (" in %s\n", inpfn);
}

savmaxseg (fp)
register int *fp;
{
  if (fp[FCODEPOS] > fp[FCODELEN])
    fp[FCODELEN] = fp[FCODEPOS];
  if (fp[FDATAPOS] > fp[FDATALEN])
    fp[FDATALEN] = fp[FDATAPOS];
  if (fp[FUDEFPOS] > fp[FUDEFLEN])
    fp[FUDEFLEN] = fp[FUDEFPOS];
}

dopass1 (hdl, fileid, libid, libofs)
int hdl, fileid, libid, libofs;
{
register int *p, *fp, curseg;
int symseg, symofs, hash;
char cmd;

  fp = &file2[file2inx++*FLAST];
  if (file2inx >= FILEMAX)
    fatal ("too many files");
  fp[FFILE] = fileid;
  fp[FLIB]  = libid;
  fp[FOFFSET]  = libofs;
  fp[FCODEPOS]  = fp[FDATAPOS]  = fp[FUDEFPOS] = 0;
  fp[FCODELEN]  = fp[FDATALEN]  = fp[FUDEFLEN] = 0;
  fp[FCODEBASE] = fp[FDATABASE] = fp[FUDEFBASE] = 0;

  curseg = CODESEG;
  if (monitor) {
    soutname (fileid, datbuf);
    printf ("processing %s", datbuf);
    if (libid != -1) {
      soutname (libid, datbuf);
      printf (" (%s)", datbuf);
    }
    printf ("\n", datbuf);
  }
  while (1) {
    if (fread (hdl, &cmd, 1) != 1) {
      printf ("missing END in %s\n", inpfn);
      return;
    }
    if (cmd < 0) {
      datlen = -cmd;
      fread (hdl, datbuf, datlen);
      if (curseg == CODESEG)
        fp[FCODEPOS] += datlen;
      else if (curseg == DATASEG)
        fp[FDATAPOS] += datlen;
      else
        fp[FUDEFPOS] += datlen;
    } else {
      switch (cmd) {
        case __ADD: case __SUB: case __MUL: case __DIV: case __MOD:
        case __LSR: case __LSL: case __XOR: case __AND: case __OR:
        case __NOT: case __NEG: case __SWAP:
          /* skip basic stack operations */
          break;
        case __PUSHB: case __CODEB: case __DATAB: case __UDEFB:
          /* skip byte length stack operations */
          fread (hdl, datbuf, 1);
          break;
        case __PUSHW: case __CODEW: case __DATAW: case __UDEFW:
          /* skip BPW length stack operations */
          fread (hdl, datbuf, BPW);
          break;
        case __SYMBOL:
          /* Push symbol value on stack */
          fread (hdl, datbuf, 1); /* length */
          datlen = datbuf[0];
          fread (hdl, datbuf, datlen); /* symbol */
          datbuf[datlen] = 0;
          dohash (datbuf, &hash);
          p = &name[hash*NLAST];
          if (!p[NTYPE])
            p[NTYPE] = UNDEF;
          if (debug)
            printf ("SYMBOL: %s\n", datbuf);
          break;
        case __POPB:
          /* increase curpos with 1 */
          if (curseg == CODESEG)
            fp[FCODEPOS] += 1;
          else if (curseg == DATASEG)
            fp[FDATAPOS] += 1;
          else
            fp[FUDEFPOS] += 1;
          break;
        case __POPW:
          /* increase curpos with BPW */
          if (curseg == CODESEG)
            fp[FCODEPOS] += BPW;
          else if (curseg == DATASEG)
            fp[FDATAPOS] += BPW;
          else
            fp[FUDEFPOS] += BPW;
          break;
        case __DSB:
	  /* skip specified number of bytes in current segment */
          fread (hdl, &symofs, BPW); /* skipcount */
          if (curseg == CODESEG)
            fp[FCODEPOS] += symofs;
          else if (curseg == DATASEG)
            fp[FDATAPOS] += symofs;
          else
            fp[FUDEFPOS] += symofs;
          break;
        case __END:
          savmaxseg (fp);
          return;
        case __CODEDEF: case __DATADEF: case __UDEFDEF:
          /* symbol definition */
          if (cmd == __CODEDEF)
            symseg = CODE;
          else if (cmd == __DATADEF)
            symseg = DATA;
          else
            symseg = UDEF;
          fread (hdl, &symofs, BPW); /* symbol offset */
          fread (hdl, datbuf, 1); /* length */
          datlen = datbuf[0];
          fread (hdl, datbuf, datlen); /* symbol */
          datbuf[datlen] = 0;
          dohash (datbuf, &hash);
          p = &name[hash*NLAST];
          if (!p[NTYPE])
            p[NTYPE] = UNDEF;
          if (p[NTYPE] == UNDEF) {
            /* new symbol */
            p[NTYPE] = symseg;
            p[NMODULE] = file2inx-1;
            p[NVALUE] = symofs;
          } else {
            printf ("Symbol '%s' doubly defined", datbuf);
            objerr (fp, "", symseg);
          }
          if (debug)
            printf ("SYMDEF: %s %d:%d\n", datbuf, symseg, symofs);
          break;
        case __CODEORG: case __DATAORG: case __UDEFORG:
          fread (hdl, &symofs, BPW); /* segment offset */
          savmaxseg (fp);
          if (cmd == __CODEORG) {
            curseg = CODESEG;
            fp[FCODEPOS] = symofs;
          } else if (cmd == __DATAORG) {
            curseg = DATASEG;
            fp[FDATAPOS] = symofs;
          } else {
            curseg = UDEFSEG;
            fp[FUDEFPOS] = symofs;
          }
          if (debug)
            printf ("ORG: %d:%d\n", curseg, symofs);
          break;
        default:
          printf ("unknown command %d", cmd);
          objerr (fp, "", curseg);
          exit (1); 
          break;
      }
    }
  }
}

dopass2 (hdl, fp)
int hdl, *fp;
{
register int i, *p, curseg, lval, rval;
int symseg, symofs, hash, val;
char cmd, cval;

  curseg = CODESEG;
  stackinx = 0;
  fseek (outhdl, fp[FCODEBASE]);

  if (monitor) {
    soutname (fp[FFILE], datbuf);
    printf ("processing %s", datbuf);
    if (fp[FLIB] != -1) {
      soutname (fp[FLIB], datbuf);
      printf (" (%s)", datbuf);
    }
    printf ("\n", datbuf);
  }

  while (1) {
    if (fread (hdl, &cmd, 1) != 1) {
      printf ("unexpected missing END in %s\n", inpfn);
      return;
    }
    if (cmd < 0) {
      datlen = -cmd;
      fread (hdl, datbuf, datlen);
      fwrite (outhdl, datbuf, datlen);
      if (curseg == CODESEG)
        fp[FCODEPOS] += datlen;
      else if (curseg == DATASEG)
        fp[FDATAPOS] += datlen;
      else {
        objerr (fp, "data in UDEF segment", curseg);
        exit (1);
      }
    } else {
      switch (cmd) {
        case __ADD: case __SUB: case __MUL: case __DIV: case __MOD:
        case __LSR: case __LSL: case __XOR: case __AND: case __OR:
          rval = stack[--stackinx];
          lval = stack[--stackinx];
          if (stackinx < 0) objerr (fp, "stack underflow", curseg);
          switch (cmd) {
            case __ADD:
              lval += rval;
              break;
            case __SUB:
              lval -= rval;
              break;
            case __MUL:
              lval *= rval;
              break;
            case __DIV: 
              lval /= rval;
              break;
            case __MOD:
              lval %= rval;
              break;
            case __LSR:
              lval >>= rval;
              break;
            case __LSL:
              lval <<= rval;
              break;
            case __XOR:
              lval ^= rval;
              break;
            case __AND:
              lval &= rval;
              break;
            case __OR:
              lval |= rval;
              break;
          }
          stack[stackinx++] = lval;
          break;
        case __NOT: case __NEG:
          lval = stack[--stackinx];
          if (stackinx < 0) objerr (fp, "stack underflow", curseg);
          if (cmd == __NOT)
            lval = ~lval;
          else
            lval = -lval;
          stack[stackinx++] = lval;
          break;
        case __SWAP:
          lval = stack[--stackinx];
          rval = stack[--stackinx];
          if (stackinx < 0) objerr (fp, "stack underflow", curseg);
          stack[stackinx++] = lval;
          stack[stackinx++] = rval;
          break;
        case __PUSHB: case __CODEB: case __DATAB: case __UDEFB:
          /* skip byte length stack operations */
          fread (hdl, datbuf, 1);
          if (cmd == __PUSHB)
            stack[stackinx] = datbuf[0];
          else if (cmd == __CODEB)
            stack[stackinx] = fp[FCODEBASE] + datbuf[0];
          else if (cmd == __DATAB)
            stack[stackinx] = fp[FDATABASE] + datbuf[0];
          else
            stack[stackinx] = fp[FUDEFBASE] + datbuf[0];
          stackinx += 1;
          if (stackinx >= STACKMAX) objerr (fp, "stack overflow", curseg);
          break;
        case __PUSHW: case __CODEW: case __DATAW: case __UDEFW:
          /* skip BPW length stack operations */
          fread (hdl, &val, BPW);
          if (cmd == __PUSHB)
            stack[stackinx] = val;
          else if (cmd == __CODEW)
            stack[stackinx] = fp[FCODEBASE] + val;
          else if (cmd == __DATAW)
            stack[stackinx] = fp[FDATABASE] + val;
          else
            stack[stackinx] = fp[FUDEFBASE] + val;
          stackinx += 1;
          if (stackinx >= STACKMAX) objerr (fp, "stack overflow", curseg);
          break;
        case __SYMBOL:
          /* Push symbol value on stack */
          fread (hdl, datbuf, 1); /* length */
          datlen = datbuf[0];
          fread (hdl, datbuf, datlen); /* symbol */
          datbuf[datlen] = 0;
          dohash (datbuf, &hash);

          stack[stackinx++] = name[hash*NLAST+NVALUE];
          if (stackinx >= STACKMAX) objerr (fp, "stack overflow", curseg);
          break;
        case __POPB:
          /* pop byte from stack */
          stackinx -= 1;
          if (stackinx < 0) objerr (fp, "stack underflow", curseg);
          if ((stack[stackinx] < -128) || (stack[stackinx] > 127))
            objerr (fp, "byte overflow", curseg);
          cval = stack[stackinx];
          fwrite (outhdl, &cval, 1);

          /* increase curpos with 1 */
          if (curseg == CODESEG)
            fp[FCODEPOS] += 1;
          else if (curseg == DATASEG)
            fp[FDATAPOS] += 1;
          else {
            objerr (fp, "data in UDEF segment", curseg);
            exit (1);
          }
          break;
        case __POPW:
          /* pop word from stack */
          stackinx -= 1;
          if (stackinx < 0) objerr (fp, "stack underflow", curseg);
          fwrite (outhdl, &stack[stackinx], BPW);

          /* increase curpos with BPW */
          if (curseg == CODESEG)
            fp[FCODEPOS] += BPW;
          else if (curseg == DATASEG)
            fp[FDATAPOS] += BPW;
          else {
            objerr (fp, "data in UDEF segment", curseg);
            exit (1);
          }
          break;
        case __DSB:
	  /* skip specified number of bytes in current segment */
          fread (hdl, &symofs, BPW); /* skipcount */
          if (curseg == CODESEG) {
            fp[FCODEPOS] += symofs;
            i = symofs;
          } else if (curseg == DATASEG) {
            fp[FDATAPOS] += symofs;
            i = symofs;
          } else {
            fp[FUDEFPOS] += symofs;
            i = 0;
          }

          datbuf[0] = datbuf[1] = datbuf[2] = datbuf[3] = 0;
          while (i > 4) {
            fwrite (outhdl, datbuf, 4);
            i -= 4;
          }
          while (i > 0) {
            fwrite (outhdl, datbuf, 1);
            i -= 1;
          }
          break;
        case __END:
          if (stackinx)
            printf ("stack not properly released in %s\n", inpfn);
          return;
        case __CODEDEF: case __DATADEF: case __UDEFDEF:
          /* symbol definition (skiped in pass2) */
          fread (hdl, &symofs, BPW); /* symbol offset */
          fread (hdl, datbuf, 1); /* length */
          datlen = datbuf[0];
          fread (hdl, datbuf, datlen); /* symbol */
          break;
        case __CODEORG: case __DATAORG: case __UDEFORG:
          fread (hdl, &symofs, BPW); /* segment offset */
          if (cmd == __CODEORG) {
            curseg = CODESEG;
            fp[FCODEPOS] = symofs;
            i = fp[FCODEBASE] + symofs;
          } else if (cmd == __DATAORG) {
            curseg = DATASEG;
            fp[FDATAPOS] = symofs;
            i = fp[FDATABASE] + symofs;
          } else {
            curseg = UDEFSEG;
            fp[FUDEFPOS] = symofs;
            i = 0;
          } 
          fseek (outhdl, i);
          break;
        default:
          printf ("unknown command %d", cmd);
          objerr (fp, "", curseg);
          exit (1); 
          break;
      }
    }
  }
}

doreloc ()
{
register int i, curpos, curlen, *p;
int hash;

  dohash ("___CODEBASE", &hash);
  name[hash*NLAST+NVALUE] = 0;
  curpos = 5;  /* Reserve space for JMP ___START */
  curlen = 0;

  /* relocate CODE */
  for (i=0; i<file2inx; i++) {
    p = &file2[i*FLAST];
    if (p[FFILE] != -1) {
      p[FCODEBASE] = curpos;
      curpos += p[FCODELEN];
      curlen += p[FCODELEN];
    }
  }

  dohash ("___CODELEN", &hash);
  name[hash*NLAST+NVALUE] = curlen;
  dohash ("___DATABASE", &hash);
  name[hash*NLAST+NVALUE] = curpos;
  curlen = 0;

  /* relocate DATA */
  for (i=0; i<file2inx; i++) {
    p = &file2[i*FLAST];
    if (p[FFILE] != -1) {
      p[FDATABASE] = curpos;
      curpos += p[FDATALEN];
      curlen += p[FDATALEN];
    }
  }

  dohash ("___DATALEN", &hash);
  name[hash*NLAST+NVALUE] = curlen;
  dohash ("___UDEFBASE", &hash);
  name[hash*NLAST+NVALUE] = curpos;
  curlen = 0;

  /* page align UDEF segment */
  curpos = (curpos + 0x01FF) & ~0x01FF;

  /* relocate UDEF */
  for (i=0; i<file2inx; i++) {
    p = &file2[i*FLAST];
    if (p[FFILE] != -1) {
      p[FUDEFBASE] = curpos;
      curpos += p[FUDEFLEN];
      curlen += p[FUDEFLEN];
    }
  }

  dohash ("___UDEFLEN", &hash);
  name[hash*NLAST+NVALUE] = curlen;

  /* redefine ___STACKLEN */
  dohash ("___STACKLEN", &hash);
  p = &name[hash*NLAST];
  p[NVALUE] = stksiz;

  /* relocate all symbols */
  for (i=0; i<NAMEMAX; i++) {
    p = &name[i*NLAST];
    if (p[NTYPE] == CODE)
      p[NVALUE] += file2[p[NMODULE]*FLAST+FCODEBASE];
    else if (p[NTYPE] == DATA)
      p[NVALUE] += file2[p[NMODULE]*FLAST+FDATABASE];
    else if (p[NTYPE] == UDEF)
      p[NVALUE] += file2[p[NMODULE]*FLAST+FUDEFBASE];
  }
}

process ()
{
register int i, j, *fp, *p, len;
int hash, found;

  pass = 1;
  printf ("Pass 1\n");

  /* process pass 1 */
  for (i=0; i<file1inx; i++) {
    fp = &file1[i*FLAST];
    if (fp[FLIB] == -1) {
      /* process object */
      soutname (fp[FFILE], inpfn);
      inphdl = mustopen (inpfn, 'R', 'B');
      dopass1 (inphdl, fp[FFILE], fp[FLIB], 0);
      fclose (inphdl);
    } else {
      /* process library */
      soutname (fp[FLIB], inpfn);
      open_olb ('RS');
      found = 0;
      while (1) {
        for (j=0; j<NAMEMAX; j++) {
          p = &name[j*NLAST];
          if (p[NTYPE] == UNDEF) {
            /* found undefined symbol, test if in library */
            soutname (j, datbuf);
            lbdohash (datbuf, &hash);
            p = &lbname[hash*LBNLAST];
            if (p[LBNLIB] != -1) {
              /* found module containing symbol definition */
              p = &lbfile[p[LBNLIB]*LBFLAST];
              fseek (inphdl, p[LBFOFFSET]);
              /* add module name to linker symboltable */
              lbsoutname (p[LBFNAME], datbuf);
              dohash (datbuf, &hash);
              /* process */
              dopass1 (inphdl, hash, fp[FLIB], p[LBFOFFSET]);
              found = 1;
            }
          }
        }
        /* if all symbols tested, test for another load */
        if (!found)
          break;
        found = 0;
      }
      fclose (inphdl);
    }
  }

  /* relocate modules */
  doreloc ();

  /* test for undefined symbols */
  if (!undef) {
    j = 0;
    for (i=0; i<NAMEMAX; i++) {
      p = &name[i*NLAST];
      if (p[NTYPE] == UNDEF) {
        if (!j) {
          printf ("Undefined symbols :\n");
          j=1;
        }
        outname (i);
        printf ("\n");
      }
    }
  }

  pass = 2;
  printf ("Pass 2\n");

  /* generate prefix "JMP ___START" */
  datbuf[0] = 0x6F;  /* opcode for JMP */
  fwrite (outhdl, datbuf, 1);
  dohash ("___START", &hash);
  fwrite (outhdl, &name[hash*NLAST+NVALUE], BPW);
  datbuf[0] = datbuf[1] = 0; /* No register indirects */
  fwrite (outhdl, datbuf, 2);

  /* process pass 2 */
  for (i=0; i<file2inx; i++) {
    fp = &file2[i*FLAST];
    if (fp[FLIB] == -1) {
      /* process object */
      soutname (fp[FFILE], inpfn);
      inphdl = mustopen (inpfn, 'R', 'B');
      dopass2 (inphdl, fp);
      fclose (inphdl);
    } else {
      /* process library */
      soutname (fp[FLIB], inpfn);
      inphdl = mustopen (inpfn, 'RS', 'B');
      while (1) {
        fseek (inphdl, fp[FOFFSET]);
        dopass2 (inphdl, fp);
        /* test if next module comes from same library */
        if (((i+1) >= file2inx) || (file2[(i+1)*FLAST+FLIB] != fp[FLIB]))
          break;
        /* reinit fp and redo loop */
        fp = &file2[++i*FLAST];
      }
      fclose (inphdl);
    }
  }
}
