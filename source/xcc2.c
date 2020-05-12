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
** X-C-Compiler.  Part 2, Syntax Control
*/

#define EXTERN extern
#include "xcc.h"

/*
** Declare/define a local symbol
*/
declloc (locscope, class)
int locscope;
register int class;
{
int size, sname, len, ptr, type, cnt;
register int *loc, i;

  /* get size of type */
  if (amatch ("char"))
    size = 1;
  else if (amatch ("int"))
    size = 2;
  else
    return 0;

  /* scan definitions */
  while (1) {
    ptr = (match ("(*") || match ("*"));
    if (!(len = dohash (lptr, &sname)))
      illname ();
    if (len)
      bump (len);
    for (i=locscope; i<locinx; i++)
      if (locsym[i*ILAST+INAME] == sname)
        multidef ();
    match (")");
    type = VARIABLE;
    if (match ("()"))
      type = FUNCTION;

    cnt = 1; /* Number of elements */
    if (match ("[")) {
      /* test for valid types */
      if (ptr || (type != VARIABLE))
        error ("array type not supported");
      ptr = 0;
      type = ARRAY;

      /* get number of elements */
      if (!constexpr (&cnt))
        cnt = 0;
      else if (cnt < 0) {
        error ("negative size illegal");
        cnt = 0;
      }

      /* Convert empty arrays into pointers */
      if (!cnt) {
        type = VARIABLE;
        ptr = 1;
      } else if (class == REGISTER)
        error ("register array not allowed");

      /* force single dimension */
      needtoken ("]");
    }

    /* add symbol to symboltable */
    if (locinx >= LOCMAX)
      fatal ("local symboltable overflow");
    loc = &locsym[locinx++ * ILAST];
    loc[INAME] = sname;
    loc[ITYPE] = type;
    loc[IPTR] = (ptr || (type == ARRAY)); /* 'true' if *<name> can be used */
    loc[ICLASS] = class;
    loc[ISIZE] = size;

    /* allocate on stack */
    if (class == REGISTER) {
      loc[IVALUE] = allocreg ();
      reglock |= (1<<loc[IVALUE]);
    } else if (ptr)
      loc[IVALUE] = (csp -= BPW);
    else if (size == 1)
      loc[IVALUE] = (csp -= cnt);
    else
      loc[IVALUE] = (csp -= cnt*BPW);

    /* test for more */
    if (!match (","))
      break;
  }

  /* done */
  ns ();
  return 1;
}

/*
** Declare/define a procedure argument
*/
declarg (class, totargc)
register int class, totargc;
{
int size, sname, len, ptr, type, cnt, reg;
register int *loc, i;

  cnt = 0;

  /* get size of type */
  if (amatch ("char"))
    size = 1; /* char is stored as int on stack */
  else if (amatch ("int"))
    size = 2;
  else
    return cnt;

  /* scan definitions */
  while (1) {
    ptr = (match ("(*") || match ("*"));
    if (!(len = dohash (lptr, &sname)))
      illname ();
    if (len)
      bump (len);
    for (i=0; i<locinx; i++) {
      loc = &locsym[i*ILAST];
      if (loc[INAME] == sname)
        break;
    }
    if (i >= locinx)
      error ("argument not defined in parameter list");
    else if (loc[ITYPE] != CONSTANT)
      multidef ();

    match (")");
    type = VARIABLE;
    if (match ("()"))
      type = FUNCTION;

    if (match ("[")) {
      /* test for valid types */
      if (ptr || (type != VARIABLE))
        error ("array type not supported");
      ptr = 0;
      type = VARIABLE;
      ptr = 1;

      /* get number of elements */
      if (constexpr (&cnt))
        error ("arraysize not allowed");

      /* force single dimension */
      needtoken ("]");
    }

    /* add symbol to symboltable */
    ++cnt;
    loc[INAME] = sname;
    loc[ITYPE] = type;
    loc[IPTR] = ptr;
    loc[ICLASS] = class;
    loc[ISIZE] = size;
    loc[IVALUE] = (totargc - loc[IVALUE] + 1) * BPW;

    /* modify location if chars */
    if ((!ptr) && (size == 1))
      loc[IVALUE] += (BPW-1);

    /* generate code for register variables */
    if (class == REGISTER) {
      reg = allocreg ();
      reglock |= (1<<reg);
      gencode_IND(((loc[LSIZE] == BPW) || loc[LPTR]) ? _LODW : _LODB, reg, loc[IVALUE], REG_AP);
      loc[IVALUE] = reg;
    }

    /* test for more */
    if (!match (","))
      break;
  }

  /* done */
  ns ();
  return cnt;
}

/*
** General global definitions
*/
declgbl (class)
register int class;
{
int size, sname, len, ptr, type, cnt;
register int *glb, i;
int lval[LLAST];

  /* get size of type */
  if (amatch ("char"))
    size = 1;
  else if (amatch ("int") || (class == EXTERNAL) || (class == STATIC))
    size = 2;
  else
    return 0;

  while (1) {
    ptr = (match ("(*") || match ("*"));
    if (!(len = dohash (lptr, &sname)))
      illname ();
    if (len)
      bump (len);
    if (findglb (sname))
      multidef ();
    if (match (")"))
      ;
    type = VARIABLE;
    if (match ("()"))
      type = FUNCTION;
    cnt = 1; /* Number of elements */
    if (match ("[")) {
      if (ptr) {
        error ("no pointer arrays");
        ptr = 0;
      }
      if (type != VARIABLE)
        error ("array type not supported");
      type = ARRAY;

      if (!constexpr (&cnt))
        cnt = 0;
      else if (cnt < 0) {
        error ("negative size illegal");
        cnt = 0;
      }

      /* Convert empty arrays into pointers */
      if (!cnt) {
        type = VARIABLE;
        ptr = 1;
      }

      needtoken ("]");      /* force single dimension */
    }

    /* Convert empty arrays into pointers */
    if ((type == ARRAY) && !cnt) {
      type = VARIABLE;
      ptr = 1;
    }

    /* add symbol to symboltable */
    if (glbinx >= GLBMAX)
      fatal ("global symboltable overflow");
    glb = &glbsym[glbinx++ * ILAST];
    glb[INAME] = sname;
    glb[ITYPE] = type;
    glb[IPTR] = (ptr || (type == ARRAY)); /* 'true' if *<name> can be used */
    glb[ICLASS] = class;
    glb[IVALUE] = 0;
    glb[ISIZE] = size;

    /* Now generate code */
    if (class == EXTERNAL) {
/* Not needed by assembler 
      fprintf (outhdl, "\t.XREF\t");
      fprintf (outhdl, "_");
      symname (sname);
      fprintf (outhdl, "\n");
*/
    } else if (match ("=")) {
      toseg (DATASEG);
      fprintf (outhdl, "_");
      symname (sname);
      fprintf (outhdl, ":");
      if (class != STATIC)
        fprintf (outhdl, ":");

      /* assign value to variable */
      litinx = 0;
      if (!match ("{")) {
        /* single value */
        if (!hier1 (lval))
          error ("expected a constant");
        if (lval[LTYPE] == CONSTANT) {
          if (ptr || (type == ARRAY))
            error ("cannot assign constant to pointer or array");
          addlits (lval[LVALUE], size);
        } else if (lval[LTYPE] == LABEL) {
          if ((size != 1) || (!ptr && (type != ARRAY)))
            error ("must assign to char pointers or arrays");
          /* at this point, literal queue has been filled with string */
        } else {
          error ("expected a constant");
          litinx = 2;
        }
      } else {
        /* multiple values */
        if (!ptr && (type != ARRAY))
          error ("must assign to pointers or arrays");

        /* get values */
        while (1) {
          --cnt;
          if (!hier1 (lval))
            error ("expected a constant");
          if (lval[LTYPE] == CONSTANT) {
            addlits (lval[LVALUE], size);
          } else if (lval[LTYPE] == LABEL) {
            error ("multiple strings not allowed");
            litinx = 2;
          } else {
            error ("expected a constant");
          }

          /* test for reloop */
          if (match (","))
            continue;
          else if (match ("}"))
            break;
          else
            error ("expected }");
        }
      }

      /* dump literal pool */
      if (ptr) {
        fprintf (outhdl, "\t.DCW\t_%d", ++nxtlabel);
        fprintf (outhdl, "_%d:", nxtlabel);
      }
      dumplits (size);

      /* if array and not all elements have been supplied, then pad */
      if (!ptr && (cnt > 0))
        if (size == 1)
          fprintf (outhdl, "\t.DSB\t%d\n", cnt);
        else
          fprintf (outhdl, "\t.DSW\t%d\n", cnt);
    } else {
      toseg (UDEFSEG);
      fprintf (outhdl, "_");
      symname (sname);
      fprintf (outhdl, ":");
      if (class != STATIC)
        fprintf (outhdl, ":");
      if (ptr)
        fprintf (outhdl, "\t.DSW\t1\n");
      else if (size == 1)
        fprintf (outhdl, "\t.DSB\t%d\n", cnt);
      else
        fprintf (outhdl, "\t.DSW\t%d\n", cnt);
    }

    /* test for more */
    if (!match (","))
      break;
  }

  /* done */
  ns ();
  return 1;
}

/*
** open an include file
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
    /* Modify sourceline to extract filename */
    p = incfn;
    gch (); /* Skip delimiter */
    while ((*p++ = gch ()) != '"') ;
    *--p = 0; /* Remove delimiter */
    
    /* Open file */
    inchdl = mustopen (incfn, "r");
    }

  /* make next read come from new file (if open) */
  kill ();
}

/*
** Copy assembler source directly to the output file
*/
doasm ()
{
  ccode = 0; /* mark mode as "asm" */
  readline (); /* skip #asm */
  while (inphdl) {
    white ();
    if (ch && amatch("#endasm"))
      break;
    fprintf (outhdl, "%s\n", line);
    readline ();
  }
  kill (); /* erase to eoln */
  ccode = 1;
}

/*
** Declare/define a macro
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
    /* copy macroname */
    mptr = &mac[macinx++ * MLAST];
    mptr[MNAME] = sname; 
    mptr[MEXPAND] = &macq[macqinx];
    /* Copy expansion */
    bump (len);
    white ();
    while (ch) {
      if (macqinx < MACQMAX)
        macq[macqinx++] = ch;
      gch ();
    }
    if (macqinx < MACQMAX)
      macq[macqinx++] = 0; /* Terminator */
    if (macqinx >= MACQMAX)
      fatal ("#define string overflow");
  }
}

/*
**
*/
newfunc ()
{
int returnlbl, len, sname, lbl1, lbl2, inireg, sav_argc;
register int *p, i, argc;

  returnlbl = ++nxtlabel;
  locinx = 0; /* reset locals */
  reguse = regsum = reglock = 1<<REG_AP; /* reset all registers */
  csp = -1; /* reset stack */
  argc = 0;
  swinx = 1;
  toseg (CODESEG);
  if (monitor)
    printf ("%s\n", lptr);

  /* get procedure name */
  if (!(len = dohash (lptr, &sname))) {
    error ("illegal function name or declaration");
    kill ();
    return;
  }
  bump (len);
  p = findglb (sname);
  if (!p) {
    if (glbinx >= GLBMAX)
      fatal ("global symboltable overflow");
    p = &glbsym[glbinx++ * ILAST];
  } else if (p[ICLASS] != AUTOEXT)
    multidef ();
  /* (re)define procedure */
  p[INAME] = sname;
  p[ITYPE] = FUNCTION;
  p[IPTR] = 0;
  p[ICLASS] = GLOBAL;
  p[IVALUE] = 0;
  p[ISIZE] = BPW;
  /* Generate global label */
  fprintf (outhdl, "_");
  symname (sname);
  fprintf (outhdl, "::");
  gencode_R (_LODR, 1, REG_SP);
  lbl1 = ++nxtlabel;
  fprintf (outhdl, "_%d:", lbl1);
  gencode_I (_PSHR, 0, 0);
  gencode_R (_LODR, REG_AP, 1);

  /* get arguments */
  if (!match ("("))
    error ("no open parent");
  blanks ();
  while (ch != ')') {
    /* get argument */
    blanks ();
    if (!(len = dohash (lptr, &sname))) {
      error ("illegal argument name");
      junk ();
    } else {
      bump (len);
      for (i=0; i<locinx; i++) {
        p = &locsym[i*ILAST];
        if (p[INAME] == sname)
          break;
      }
      if (i < locinx)
        multidef ();
      else if (locinx >= LOCMAX)
        fatal ("local symboltable overflow");
      else
        p = &locsym[locinx++ * ILAST];
      /* define argument */
      p[INAME] = sname;
      p[ITYPE] = CONSTANT;  /* Mark as undefined */
      p[IPTR] = 0;
      p[ICLASS] = AP_AUTO;
      p[IVALUE] = argc; /* argument ID (starts at 0) */
      p[ISIZE] = BPW;
      ++argc;
    }

    if (!match (","))
      break;
  }
  needtoken (")");

  /* now define arguments */
  sav_argc = argc;
  while (1) {
    if (amatch ("register"))
      argc -= declarg (REGISTER, sav_argc);
    else if (i = declarg (AP_AUTO, sav_argc))
      argc -= i;
    else if (argc) {
      error ("wrong number of arguments");
      break;
    } else
      break;
  }

  /* get statement */
  inireg = reglock;
  statement (swinx, returnlbl, 0, 0, csp, csp);
  if (csp != -1)
    error ("internal error. stack not released");
  if (reglock != inireg)
    error ("internal error. registers not unlocked");

  /* trailing statements */
  lbl2 = ++nxtlabel;
  fprintf (outhdl, "_%d:\t.ORG\t_%d\n", lbl2, lbl1);
  gencode_I (_PSHR, 0, regsum);
  fprintf (outhdl, "\t.ORG\t_%d\n", lbl2);
  fprintf (outhdl, "_%d:", returnlbl);
  gencode_I (_POPR, 0, regsum);
  gencode (_RSB);
}


/*
** process one statement
*/
statement (swbase, returnlbl, breaklbl, contlbl, breaksp, contsp)
int swbase, returnlbl, breaklbl, contlbl, breaksp, contsp;
{
int lval[LLAST], sav_loc, sav_sw;
register int sav_csp, i, *ptr;
int lbl1, lbl2, lbl3;

  if (debug)
    fprintf (outhdl, ";S %s\n", lptr);

  if (match ("{")) {
    sav_csp = csp;
    sav_loc = locinx;
    while (!match ("}")) {
      if (ch <= ' ')
        blanks ();
      if (!ch) {
        error ("no closing }"); /* EOF */
        return;
      } else if (amatch ("register")) {
        declloc (sav_loc, REGISTER);
        if (sav_csp > 0)
          error ("Definitions not allowed here");
      } else if (declloc (sav_loc, SP_AUTO)) {
        if (sav_csp > 0)
          error ("Definitions not allowed here");
      } else {
        /* allocate locals */
        if ((sav_csp < 0) && (csp != sav_csp)) {
          gencode_ADJSP (csp-sav_csp);
          sav_csp = -sav_csp;
        }
        statement (swbase, returnlbl, breaklbl, contlbl, breaksp, contsp);
      }
    }
    /* done */
    if (sav_csp > 0)
      sav_csp = -sav_csp;
    if (csp != sav_csp) {
      gencode_ADJSP (sav_csp-csp);
      csp = sav_csp;
    }
    /* free local registers */
    for (i=sav_loc; i<locinx; i++) {
      ptr = &locsym[i*ILAST];
      if (ptr[ICLASS] == REGISTER)
        freereg (ptr[IVALUE]);
    }
    locinx = sav_loc;
    return;
  }

  if (amatch ("if")) {
    needtoken ("(");
    expression (lval, 1);
    needtoken (")");
    if (lval[LTYPE] == BRANCH) {
      if (!lval[LFALSE])
        lval[LFALSE] = ++nxtlabel;
      gencode_L (lval[LVALUE], lval[LFALSE]);
    } else {
      lval[LFALSE] = ++nxtlabel;
      lval[LTRUE] = 0;
      loadlval (lval, 0);
      gencode_L (_EQ, lval[LFALSE]);
    }
    if (lval[LTRUE])
      fprintf (outhdl, "_%d:", lval[LTRUE]);
    freelval (lval);
    statement (swbase, returnlbl, breaklbl, contlbl, breaksp, contsp);
    if (!amatch ("else")) {
      fprintf (outhdl, "_%d:", lval[LFALSE]);
    } else {
      lbl1 = ++nxtlabel;
      gencode_L (_JMP, lbl1);
      fprintf (outhdl, "_%d:", lval[LFALSE]);
      statement (swbase, returnlbl, breaklbl, contlbl, breaksp, contsp);
      fprintf (outhdl, "_%d:", lbl1);
    }
  } else if (amatch ("while")) {
    lbl1 = ++nxtlabel;
    fprintf (outhdl, "_%d:", lbl1);
    needtoken ("(");
    expression (lval, 1);
    needtoken (")");

    if (lval[LTYPE] == BRANCH) {
      if (!lval[LFALSE])
        lval[LFALSE] = ++nxtlabel;
      gencode_L (lval[LVALUE], lval[LFALSE]);
    } else {
      lval[LFALSE] = ++nxtlabel;
      lval[LTRUE] = 0;
      loadlval (lval, 0);
      gencode_L (_EQ, lval[LFALSE]);
    }
    if (lval[LTRUE])
      fprintf (outhdl, "_%d:", lval[LTRUE]);
    freelval (lval);
    statement (swbase, returnlbl, lval[LFALSE], lbl1, csp, csp);
    gencode_L (_JMP, lbl1);
    fprintf (outhdl, "_%d:", lval[LFALSE]);
  } else if (amatch ("do")) {
    lbl1 = ++nxtlabel;
    lbl2 = ++nxtlabel;
    fprintf (outhdl, "_%d:", lbl1);
    statement (swbase, returnlbl, lbl2, lbl1, csp, csp);
    fprintf (outhdl, "_%d:", lbl2);
    needtoken ("while");
    needtoken ("(");
    expression (lval, 1);
    needtoken (")");
    if (lval[LTYPE] == BRANCH) {
      if (lval[LTRUE])
        fprintf (outhdl, "_%d=_%d\n", lval[LTRUE], lbl1);
      else
        lval[LTRUE] = lbl1;
      gencode_L (negop (lval[LVALUE]), lval[LTRUE]);
      if (lval[LFALSE])
        fprintf (outhdl, "_%d:", lval[LFALSE]);
    } else {
      loadlval (lval, 0);
      gencode_L (_NE, lbl1);
    }
    freelval (lval);
  } else if (amatch ("for")) {
    lbl1 = ++nxtlabel;
    lbl2 = ++nxtlabel;
    needtoken ("(");
    if (ch <= ' ')
      blanks ();
    if (ch != ';') {
      expression (lval, 1); 
      freelval (lval);
    }
    needtoken (";");
    fprintf (outhdl, "_%d:", lbl1);
    if (ch <= ' ')
      blanks ();
    if (ch != ';') {
      expression (lval, 1); 
      if (lval[LTYPE] == BRANCH) {
        if (!lval[LFALSE])
          lval[LFALSE] = ++nxtlabel;
        if (!lval[LTRUE])
          lval[LTRUE] = ++nxtlabel;
        gencode_L (lval[LVALUE], lval[LFALSE]);
      } else {
        lval[LFALSE] = ++nxtlabel;
        lval[LTRUE] = ++nxtlabel;
        loadlval (lval, 0);
        gencode_L (_EQ, lval[LFALSE]);
      }
      freelval (lval);
    }
    gencode_L (_JMP, lval[LTRUE]);
    needtoken (";");
    fprintf (outhdl, "_%d:", lbl2);
    if (ch <= ' ')
      blanks ();
    if (ch != ';') {
      expression (lval, 1); 
      freelval (lval);
      gencode_L (_JMP, lbl1);
    }
    needtoken (")");
    fprintf (outhdl, "_%d:", lval[LTRUE]);
    statement (swbase, returnlbl, lval[LFALSE], lbl1, csp, csp);
    gencode_L (_JMP, lbl2);
    fprintf (outhdl, "_%d:", lval[LFALSE]);
  } else if (amatch ("switch")) {
    needtoken ("(");
    expression (lval, 1); 
    needtoken (")");
    loadlval (lval, 1);
    lbl1 = ++nxtlabel;
    lbl2 = ++nxtlabel;
    gencode_L (_JMP, lbl1);
    sav_sw = swinx;
    if (swinx >= SWMAX)
      fatal ("switch table overflow");
    sw[swinx++*SLAST+SLABEL] = 0; /* enable default */
    statement (sav_sw, returnlbl, lbl2, contlbl, csp, contsp);
    gencode_L (_JMP, lbl2);
    dumpsw (sav_sw, lbl1, lbl2);
    fprintf (outhdl, "_%d:", lbl2);
    swinx = sav_sw; 
  } else if (amatch ("case")) { 
    if (!swbase)
      error ("not in switch");
    if (!constexpr (&lbl3))
      error ("case value expected");
    needtoken (":");    
    for (i=swbase+1; i<swinx; i++)
      if (sw[i*SLAST+SCASE] == lbl3)
        error ("case value already defined");
    lbl1 = ++nxtlabel;
    fprintf (outhdl, "_%d:", lbl1);
    if (swinx >= SWMAX)
      fatal ("switch table overflow");
    ptr = &sw[swinx++*SLAST];
    ptr[SCASE] = lbl3;
    ptr[SLABEL] = lbl1;
  } else if (amatch ("default")) {
    if (!swbase)
      error ("not in switch");
    needtoken (":");    
    ptr = &sw[swbase*SLAST];
    if (ptr[SLABEL])
      error ("multiple defaults");
    lbl1 = ++nxtlabel;
    fprintf (outhdl, "_%d:", lbl1);
    ptr[SLABEL] = lbl1;
  } else if (amatch ("return")) { 
    if (!endst ()) {
      /* generate a return value in R1 */
      expression (lval, 1);
      loadlval (lval, 1);
    }
    if (csp != -1)
      gencode_ADJSP (-1 - csp);
    gencode_L (_JMP, returnlbl);
    ns ();
  } else if (amatch ("break")) {
    if (!breaklbl)
      error ("not in block");
    if (csp != breaksp)
      gencode_ADJSP (breaksp-csp);
    gencode_L (_JMP, breaklbl);
    ns ();
  } else if (amatch ("continue")) {
    if (!contlbl)
      error ("not in block");
    if (csp != contsp)
      gencode_ADJSP (contsp-csp);
    gencode_L (_JMP, contlbl);
    ns ();
  } else if (!ch) {
    return; /* EOF */
  } else if (amatch ("#asm")) {
    doasm();
  } else if (ch != ';') {
    expression (lval, 1);
    freelval (lval);
    ns ();
  } else
    ns ();
}

/*
** Dump the switch map and decoding
*/
dumpsw (swbase, codlbl, endlbl)
int swbase, codlbl, endlbl;
{
register int lo, hi, i, j, cnt, *ptr;
int maplbl, deflbl, lbl, lval[LLAST];

  if (swbase+1 == swinx) {
    /* no cases specified */
    ptr = &sw[swbase*SLAST];
    if (ptr[SLABEL])
      gencode_L (_JMP, ptr[SLABEL]);
    return;
  }

  /* get lo/hi bounds */
  ptr = &sw[(swbase+1)*SLAST];
  lo = hi = ptr[SCASE];
  for (i=swbase+1; i<swinx; i++) {
    ptr = &sw[i*SLAST];
    if (ptr[SCASE] > hi)
      hi = ptr[SCASE];
    if (ptr[SCASE] < lo)
      lo = ptr[SCASE];
  }

  /* setup default */
  if (!(deflbl = sw[swbase*SLAST+SLABEL]))
    deflbl = endlbl;

  /* generate map */
  maplbl = ++nxtlabel;
  toseg (DATASEG);
  fprintf (outhdl, "_%d:", maplbl);
  cnt = 0;
  for (i=lo; i<=hi; i++) {
    lbl = deflbl;
    for (j=swbase+1; j<swinx; j++) {
      ptr = &sw[j*SLAST];
      if (ptr[SCASE] == i) {
        lbl = ptr[SLABEL];
        break;
      }
    }
    if (!cnt++)
      fprintf (outhdl, "\t.DCW\t_%d", lbl);
    else
      fprintf (outhdl, ",_%d", lbl);
    if (cnt > 15) {
      fprintf (outhdl, "\n");
      cnt = 0;
    }      
  }
  if (cnt)
    fprintf (outhdl, "\n");
  toseg (prevseg);

  /* generate code (use j as reg) */
  fprintf (outhdl, "_%d:", codlbl);
  j = allocreg ();
  gencode_I (_LEA,  j, lo);
  gencode_R (_SUB,  1, j);
  gencode_L (_LT,   deflbl);
  gencode_I (_LEA,  j, hi-lo);
  gencode_R (_CMP,  1, j);
  gencode_L (_GT,   deflbl);
  gencode_R (_MUL,  1, REG_BPW);
  lval[LTYPE] = LABEL;
  lval[LNAME] = maplbl;
  lval[LREG1] = 1;
  lval[LREG2] = lval[LVALUE] = 0;
  gencode_M (_LODW, j, lval);
  gencode_IND(_JMP, 0, 0, j);
  freereg (j);
}

/*
** process all input text
**
** At this level, only static declarations,
**      defines, includes and function
**      definitions are legal...
*/
parse ()
{
  blanks ();
  while (inphdl) {
    if (amatch ("extern"))
      declgbl (EXTERNAL);
    else if (amatch ("static"))
      declgbl (STATIC);
    else if (amatch ("register")) {
      error ("global register variables not allowed");
      declgbl (GLOBAL);
    } else if (declgbl (GLOBAL))
      ;
    else if (amatch ("#asm"))
      doasm();
    else if (amatch ("#include"))
      doinclude ();
    else if (amatch ("#define"))
      declmac ();
    else
      newfunc ();
    blanks ();
  }
}

