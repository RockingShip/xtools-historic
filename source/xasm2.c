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
** X-Assembler.  Part 2, Syntax control
*/

#define EXTERN extern
#include "xasm.h"

get_comma ()
{
  if (ch <= ' ') white ();
  if (ch == ',')
    gch ();
  else
    error ("expected comma");
}

get_reg ()
{
int hash, reg;
register int len, *p;

  if (ch <= ' ')
    white ();
  if (!ch)
    len = 0;
  else
    len = dohash (lptr, &hash);
  if (!len) {
    error ("expected register");
    junk ();
    reg = 0;
  } else {
    bump (len);
    while (1) {
      p = &name[hash*NLAST];
      if (p[NTYPE] != LINK)
        break;
      else
        hash = p[NVALUE];
    }
    if (p[NTYPE] != REGISTER) {
      error ("expected register");
      reg = 0;
    } else 
      reg = p[NVALUE];
  }
  /* process reg */
  sto_data (reg, 1);
}

get_imm ()
{
int lval[LLAST];

  expression (lval);
  if (lval[LTYPE] == CONSTANT)
    sto_data (lval[LVALUE], BPW);
  else {
    loadlval (lval);
    sto_cmd (__POPW, 0);
  }
}

get_mem ()
{
int lval[LLAST];

  if (ch <= ' ') white ();
  if (ch != '(')
    get_imm ();
  else
    sto_data (0, BPW); /* no address */

  /* test for registers */
  if (ch <= ' ') white ();
  if (ch != '(') {
    sto_data (0, 1); /* dummy lreg */
    sto_data (0, 1); /* dummy rreg */
  } else {
    gch (); if (ch <= ' ') white ();
    if ((ch == ',') || (ch == ')'))
      sto_data (0, 1); /* dummy lreg */
    else
      get_reg ();
    if (ch <= ' ') white ();
    if (ch == ',') {
      gch (); if (ch <= ' ') white ();
      if (ch == ')')
        sto_data (0, 1); /* dummy rreg */
      else
        get_reg ();
    } else
      sto_data (0, 1); /* dummy rreg */
    if (ch <= ' ') white ();
    if (ch == ')')
      gch ();
    else
      error ("expected )");
  }
}

do_opcode (p)
register int p[];
{
  if (pass == 1) {
    switch (p[NVALUE]) {
      case _ILLEGAL: case _RSB:
        curpos[curseg] += 1;
        break;
      case _PSHR: case _POPR: case _SVC:
        curpos[curseg] += 3;
        break;
      case _PSHB: case _PSHW: case _PSHA:
      case _JSB:
        curpos[curseg] += 5;
        break;
      case _NEG: case _NOT:
        curpos[curseg] += 2;
        break;
      case _ADD: case _SUB: case _MUL:  case _DIV: case _MOD:
      case _BOR: case _XOR: case _BAND: case _LSR: case _LSL:
      case _LODR: case _CMP:
        curpos[curseg] += 3;
        break;
      case _EQ: case _NE: case _LT: 
      case _LE: case _GT: case _GE: 
      case _JMP:
        curpos[curseg] += 5;
        break;
      case _LODB: case _LODW: case _LEA: 
      case _STOB: case _STOW:
        curpos[curseg] += 6;
        break;
      default:
        error ("unimplemented opcode");
        break;
    }
    kill ();
  } else {
    sto_data (p[NVALUE], 1);
    switch (p[NVALUE]) {
      case _ILLEGAL: case _RSB:
        curpos[curseg] += 1;
        break;
        break;
      case _PSHR: case _POPR: case _SVC:
        get_imm ();
        curpos[curseg] += 3;
        break;
      case _PSHB: case _PSHW: case _PSHA:
      case _JSB:
        get_mem ();
        curpos[curseg] += 5;
        break;
      case _NEG: case _NOT:
        get_reg ();
        curpos[curseg] += 2;
        break;
      case _ADD: case _SUB: case _MUL:  case _DIV: case _MOD:
      case _BOR: case _XOR: case _BAND: case _LSR: case _LSL:
      case _LODR: case _CMP:
        get_reg ();
        get_comma ();
        get_reg ();
        curpos[curseg] += 3;
        break;
      case _EQ: case _NE: case _LT: 
      case _LE: case _GT: case _GE: 
      case _JMP:
        get_mem ();
        curpos[curseg] += 5;
        break;
      case _LODB: case _LODW: case _LEA: 
      case _STOB: case _STOW:
        get_reg ();
        get_comma ();
        get_mem ();
        curpos[curseg] += 6;
        break;
      default:
        error ("unimplemented opcode");
        kill ();
        break;
    }
  }
}

savmaxpos ()
{
  if (curpos[CODESEG] > maxpos[CODESEG])
    maxpos[CODESEG] = curpos[CODESEG];
  if (curpos[DATASEG] > maxpos[DATASEG])
    maxpos[DATASEG] = curpos[DATASEG];
  if (curpos[UDEFSEG] > maxpos[UDEFSEG])
    maxpos[UDEFSEG] = curpos[UDEFSEG];
}

do_pseudo (p)
register int p[];
{
int val, lval[LLAST];
register int size;

  switch (p[NVALUE]) {
    case _CODE:
      curseg = CODESEG;
      sto_cmd (__CODEORG, curpos[CODESEG]);
      break;
    case _DATA:
      curseg = DATASEG;
      sto_cmd (__DATAORG, curpos[DATASEG]);
      break;
    case _UDEF:
      curseg = UDEFSEG;
      sto_cmd (__UDEFORG, curpos[UDEFSEG]);
      break;
    case _DSB: case _DSW:
      size = p[NVALUE] == _DSB ? 1 : BPW;
      if (!constexpr (&val)) {
        if (pass == 1)
          error ("constant required");
      } else 
        curpos[curseg] += val*size;
      sto_cmd (__DSB, val*size);
      break;
    case _DCB: case _DCW:
      size = p[NVALUE] == _DCB ? 1 : BPW;
      while (1) {
        if (match ("\"")) {
          while (ch && (ch != '"')) {
            val = litchar ();
            if (pass == 2)
              sto_data (val, size);
            curpos[curseg] += size;
          }
          gch (); /* skip terminator */
        } else {
          expression (lval);
          if (pass == 2) {
            if (lval[LTYPE] != CONSTANT) {
              loadlval (lval);
              if (size == 1)
                sto_cmd (__POPB, 0);
              else
                sto_cmd (__POPW, 0);
            } else if (size == BPW)
              sto_data (lval[LVALUE], BPW);
            else if ((lval[LVALUE] >= -128) && (lval[LVALUE] <= 127))
              sto_data (lval[LVALUE], 1);
            else
              error ("constant out of range");
          }
          curpos[curseg] += size;
        }
        if (!match (","))
          break;
      }
      break;
    case _ORG:
      savmaxpos ();
      expression (lval);
      if (lval[LTYPE] == CONSTANT) {
        curpos[curseg] = lval[LVALUE];
      } else if (lval[LTYPE] == SYMBOL) {
        p = &name[lval[LVALUE]*NLAST];
        switch (p[NTYPE]) {
          case CODE:
            curseg = CODESEG;
            break;
          case DATA:
            curseg = DATASEG;
            break;
          case UDEF:
            curseg = UDEFSEG;
            break;
        }
        curpos[curseg] = p[NVALUE];
      } else
        error ("Invalid address");
      if (pass == 2) {
        switch (curseg) {
          case CODESEG:
            sto_cmd (__CODEORG, curpos[curseg]);
            break;
          case DATASEG:
            sto_cmd (__DATAORG, curpos[curseg]);
            break;
          case UDEFSEG:
            sto_cmd (__UDEFORG, curpos[curseg]);
            break;
        }
      }
      break;
    default:
      error ("-> pseudo");
      kill ();
      break;
  }
}

parse ()
{
int i, len, hash, ext;
register int *p;
int lval[LLAST];

  while (inphdl) {
    if (amatch ("#include")) {
      doinclude ();
      continue;
    }
    if (amatch ("#define")) {
      declmac ();
      continue;
    }

    if (debug && (pass == 2))
      fprintf (outhdl, ";%s\n", lptr);
    while (1) {
      if (ch <= ' ')
        white ();
      if (!ch) 
        break; /* end of line */
      len = dohash (lptr, &hash);
      if (!len) {
        if (pass == 1)
          error ("expected opcode");
        kill ();
        break;
      }
      bump (len);
      p = &name[hash*NLAST];
      if (!p[NTYPE])
        p[NTYPE] = UNDEF;

      switch (p[NTYPE]) {
        case UNDEF:
          if (match (":")) {
            ext = match (":");
            if (monitor && ext) 
              printf ("%s\n", line);
            if (curseg == CODESEG) {
              p[NTYPE] = CODE;
              p[NVALUE] = curpos[CODESEG];
            } else if (curseg == DATASEG) {
              p[NTYPE] = DATA;
              p[NVALUE] = curpos[DATASEG];
            } else if (curseg == UDEFSEG) {
              p[NTYPE] = UDEF;
              p[NVALUE] = curpos[UDEFSEG];
            } else
              error ("unknown segment");
            continue;
          } else if (match ("=")) { 
            len = dohash (lptr, &p[NVALUE]);
            if (!len) {
              if (pass == 1)
                error ("use #define");
              kill ();
            } else {
              bump (len);
              p[NTYPE] = LINK;
              p = &name[p[NVALUE]*NLAST];
              if (!p[NTYPE])
                p[NTYPE] = UNDEF; /* Initial value */
            }
            break;
          } else { 
            if (pass == 1)
              error ("unknown opcode"); 
            kill ();
            break;
          }
        case ABS: case CODE: case DATA: case UDEF:
          if (match (":")) {
            ext = match (":");
            if (monitor && ext) 
              printf ("%s\n", line);
            if (p[NTYPE] == CODE) {
              if ((curseg != CODESEG) || (p[NVALUE] != curpos[CODESEG]))
                if (pass == 1)
                  error ("multiply defined");
                else
                  error ("phase error");
              if (ext && (pass == 2))
                sto_cmd (__CODEDEF, hash);
            } else if (p[NTYPE] == DATA) {
              if ((curseg != DATASEG) || (p[NVALUE] != curpos[DATASEG]))
                if (pass == 1)
                  error ("multiply defined");
                else
                  error ("phase error");
              if (ext && (pass == 2))
                sto_cmd (__DATADEF, hash);
            } else if (p[NTYPE] == UDEF) {
              if ((curseg != UDEFSEG) || (p[NVALUE] != curpos[UDEFSEG]))
                if (pass == 1)
                  error ("multiply defined");
                else
                  error ("phase error");
              if (ext && (pass == 2))
                sto_cmd (__UDEFDEF, hash);
            } else {
              error ("not implemented");
            }
            continue;
          } else if (match ("=")) { 
            if (pass == 1)
              error ("multiply defined");
            else
              expression (lval);
            break;
          } else { 
            error ("internal error"); 
            kill ();
            break;
          }
        case OPCODE:
          do_opcode (p);
          break;
        case PSEUDO:
          if (p[NVALUE] == _END)
            return; /* DONE */
          do_pseudo (p);
          break;
        case LINK:
          if (pass == 1)
            error ("multiply defined");
          kill (); /* errors displayed in pass 1 */
          break;
        default:
          if (pass == 1)
            error ("expected opcode");
          break;
      }

      /* test for junk */
      if (ch <= ' ')
        white ();
      if (ch)
        error ("encountered junk");
      break;
    }
    preprocess ();  /* fetch next line */
  }
}
