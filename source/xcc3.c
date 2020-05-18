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
//* X-C-Compiler.  Part 3, Expression evaluation
//*

#define EXTERN extern
#include "xcc.h"

/* test if lval is stored in a word */

#define lval_ISBPW  (lval[LPTR]  || (lval[LSIZE] == BPW))
#define lval2_ISBPW (lval2[LPTR] || (lval2[LSIZE] == BPW))
#define dest_ISBPW  (dest[LPTR]  || (dest[LSIZE] == BPW))
#define lval_ISIPTR (lval[LPTR]  && (lval[LSIZE] == BPW))

/*
 * Get the inverse of an compare
 */
negop (op)
register int op;
{
  switch (op) {
    case _EQ:
      return _NE;
    case _NE:
      return _EQ;
    case _GT:
      return _LE;
    case _LT:
      return _GE;
    case _GE:
      return _LT;
    case _LE:
      return _GT;
    default:
      return op; // No negation
  }
}

/*
 * Process a constant evaluation
 */
calc (left, oper, right)
register int left, right;
int oper;
{
  switch(oper) {
    case _LOR:  return (left  || right);
    case _BOR:  return (left  |  right);
    case _XOR:  return (left  ^  right);
    case _LAND: return (left  && right);
    case _BAND: return (left  &  right);
    case _EQ:   return (left  == right);
    case _NE:   return (left  != right);
    case _LE:   return (left  <= right);
    case _GE:   return (left  >= right);
    case _LT:   return (left  <  right);
    case _GT:   return (left  >  right);
    case _LSR:  return (left  >> right);
    case _LSL:  return (left  << right);
    case _ADD:  return (left  +  right);
    case _SUB:  return (left  -  right);
    case _MUL:  return (left  *  right);
    case _DIV:  return (left  /  right);
    case _MOD:  return (left  %  right);
    default:    return 0;
    }
  }

/*
 * Allocate a free register
 */
allocreg ()
{
register int i, mask;  

  for (i=2; i<REG_0; i++) {
    mask = 1<<i;
    if (!(reguse&mask)) {
      regsum |= reguse |= mask;
      return i;
    }
  }
  error ("out of registers");
  return 2; // First modifiable reg
}

/*
 * Return a register into the free list
 */
freereg (reg)
register int reg;
{
register int mask;

  mask = 1<<reg;
  if (!(REG_RESVD & mask)) {
    if (reguse & mask) {
      reguse &= ~mask;
      reglock &= ~mask;
    } else 
      error ("double register deallocation");
  }
}

/*
 * Load a lval into a register
 *
 * If 'reg' == -1 then register is mode is 'register read'. This
 * mode is same as 'reg' == 0 except reserved registers are not
 * copied to 'free' registers
 */
loadlval (lval, reg)
register int lval[], reg;
{
register int srcreg;

  // Sign extend to fix being called with negative constant when copiled with "-Dint=long"
  reg |= -(reg & (1<<SBIT));

  if (lval[LTYPE] == CONSTANT) {
    // test for a predefined register
    if (reg > 0)
      gencode_I (_LEA, reg , lval[LVALUE]);
    else {
      if (lval[LVALUE] == 0)
        srcreg = REG_0;
      else if (lval[LVALUE] == 1)
        srcreg = REG_1;
      else if (lval[LVALUE] == BPW)
        srcreg = REG_BPW;
      else if (lval[LVALUE] == 4)
        srcreg = REG_4;
      else {
        srcreg = allocreg ();
        gencode_I (_LEA, srcreg , lval[LVALUE]);
      }

      if (reg == -1)
        reg = srcreg;
      else if (srcreg < REG_0)
        reg = srcreg;
      else {
        reg = allocreg ();
        gencode_R (_LODR, reg , srcreg);
      }
    }

    // Modify lval
    lval[LTYPE] = EXPR;
    lval[LPTR]  = 0;
    lval[LEA]   = EA_REG;
    lval[LNAME] = lval[LVALUE] = 0;
    lval[LREG1] = reg;
    lval[LREG2] = 0;
  } else if (lval[LTYPE] == BRANCH) {
    int lblX;
    lblX = ++nxtlabel;

    if (reg <= 0)
      reg = allocreg ();
    if (!lval[LFALSE])
      lval[LFALSE] = ++nxtlabel;
    gencode_L (lval[LVALUE], lval[LFALSE]);
    if (lval[LTRUE])
      fprintf (outhdl, "_%d:", lval[LTRUE]);
    gencode_R (_LODR, reg, REG_1);
    gencode_L (_JMP, lblX);
    fprintf (outhdl, "_%d:", lval[LFALSE]);
    gencode_R (_LODR, reg, REG_0);
    fprintf(outhdl, "_%d:", lblX);

    lval[LTYPE] = EXPR;
    lval[LPTR]  = 0;
    lval[LEA]   = EA_REG;
    lval[LNAME] = lval[LVALUE] = 0;
    lval[LREG1] = reg;
    lval[LREG2] = 0;
  } else if (lval[LEA] == EA_IND) {
    freelval (lval);
    if (reg <= 0)
      reg = allocreg ();
    gencode_M (lval_ISBPW ? _LODW : _LODB, reg, lval);

    lval[LEA]   = EA_REG;
    lval[LNAME] = lval[LVALUE] = 0;
    lval[LREG1] = reg;
    lval[LREG2] = 0;
  } else if (lval[LEA] != EA_REG) {
    freelval (lval);
    if (reg <= 0)
      reg = allocreg ();
    gencode_M (_LEA, reg, lval);

    lval[LEA]   = EA_REG;
    lval[LNAME] = lval[LVALUE] = 0;
    lval[LREG1] = reg;
    lval[LREG2] = 0;
  } else if (((reg > 0) && (lval[LREG1] != reg)) ||
             (REG_RESVD&(1<<lval[LREG1])) ||
             ((reg != -1) && (reglock&(1<<lval[LREG1])))) {
    freelval (lval);
    if (reg <= 0)
      reg = allocreg ();
    gencode_R (_LODR, reg, lval[LREG1]);

    lval[LREG1] = reg;
  }
}

/*
 * Free all registers assigned to a lval
 */
freelval (lval)
register int lval[];
{
  if ((lval[LTYPE] == CONSTANT) || (lval[LTYPE] == BRANCH))
    return;
  if (!(reglock & (1<<lval[LREG1])))
    freereg (lval[LREG1]);
  if (!(reglock & (1<<lval[LREG2])))
    freereg (lval[LREG2]);
}



/*
 * generic processing for <lval> { <operation> <rval> }
 */
xplng1 (hier, start, lval)
register int (*hier)(), start, lval[];
{
register char *cptr, entry;
int rval[LLAST];

  // Load lval
  if (!(*hier)(lval))
    return 0;

  while (1) {
    // Locate operation
    entry = start;
    while (1) {
      if (!(cptr = hier_str[entry]))
        return 1;
      if (omatch (cptr))
        break;
      ++entry;
    }

    // Put lval into a register
    if (!lval[LPTR] && (lval[LTYPE] == FUNCTION))
      error ("Invalid function use");

    // Load rval
    if (!(*hier)(rval)) {
      exprerr ();
      return 1;
    }

    // Put rval into a register
    if (!rval[LPTR] && (rval[LTYPE] == FUNCTION))
      error ("Invalid function use");

    // Generate code
    if ((lval[LTYPE] == CONSTANT) && (rval[LTYPE] == CONSTANT)) {
      lval[LVALUE] = calc (lval[LVALUE], hier_oper[entry], rval[LVALUE]);
    } else {
      loadlval (lval, 0);
      loadlval (rval, -1);

      // Execute operation and release rval
      gencode_R (hier_oper[entry], lval[LREG1], rval[LREG1]);
      freelval (rval);

      // Modify lval
      lval[LTYPE] = EXPR;
      lval[LPTR] = 0;
      lval[LEA] = EA_REG;
    }
  }
}

/*
 * generic processing for <lval> <comparison> <rval>
 */
xplng2 (hier, start, lval)
register int (*hier)(), start, lval[];
{
register char *cptr, entry;
int rval[LLAST];

  // Load lval
  if (!(*hier)(lval))
    return 0;

  // Locate operation
  entry = start;
  while (1) {
    if (!(cptr = hier_str[entry]))
      return 1;
    if (omatch (cptr))
      break;
    ++entry;
  }

  // Load rval
  if (!(*hier)(rval)) {
    exprerr ();
    return 1;
  }

  // Generate code
  if ((lval[LTYPE] == CONSTANT) && (rval[LTYPE] == CONSTANT)) {
    lval[LVALUE] = calc (lval[LVALUE], hier_oper[entry], rval[LVALUE]);
  } else {
    loadlval (lval, -1);
    loadlval (rval, -1);

    // Compare and release values
    gencode_R (_CMP, lval[LREG1], rval[LREG1]);
    freelval (lval);
    freelval (rval);

    // Change lval to "BRANCH"
    lval[LTYPE] = BRANCH;
    lval[LVALUE] = negop (hier_oper[entry]);
    lval[LFALSE] = lval[LTRUE] = 0;
  }
  return 1;
}

/*
 * generic processing for <lval> { ('||' | '&&') <rval> }
 */
xplng3 (hier, start, lval)
register int (*hier)(), start, lval[];
{
register char *cptr, entry;
register int lbl;
int once;

  // Load lval
  if (!(*hier)(lval))
    return 0;

  once = 1;
  entry = start;
  while (1) {
    // Locate operation
    while (1) {
      if (!(cptr = hier_str[entry]))
        return 1;
      if (omatch (cptr))
        break;
      ++entry;
    }

    // Put lval into a register
    if (!lval[LPTR] && (lval[LTYPE] == FUNCTION))
      error ("Invalid function use");

    if (once) {
      // One time only: process lval and jump

      // lval must be BRANCH
      if (lval[LTYPE] != BRANCH) {
        loadlval (lval, 0);
        freelval (lval);
        lval[LTYPE] = BRANCH;
        lval[LVALUE] = _EQ;
        lval[LFALSE] = lval[LTRUE] = 0;
      }

      if (hier_oper[entry] == _LAND) {
        if (!lval[LFALSE])
          lval[LFALSE] = ++nxtlabel;
        lbl = lval[LFALSE];
      } else {
        if (!lval[LTRUE])
          lval[LTRUE] = ++nxtlabel;
        lbl = lval[LTRUE];
      }

      // Mark done
      once = 0;
    }

    // postprocess last lval
    if (hier_oper[entry] == _LAND) {
      gencode_L (lval[LVALUE], lval[LFALSE]);
      if (lval[LTRUE])
        fprintf (outhdl, "_%d:", lval[LTRUE]);
    } else {
      gencode_L (negop (lval[LVALUE]), lval[LTRUE]);
      if (lval[LFALSE])
        fprintf (outhdl, "_%d:", lval[LFALSE]);
    }

    // Load next lval
    if (!(*hier)(lval)) {
      exprerr ();
      return 1;
    }

    // Put lval into a register
    if (!lval[LPTR] && (lval[LTYPE] == FUNCTION))
      error ("Invalid function use");

    // lval must be BRANCH
    if (lval[LTYPE] != BRANCH) {
      loadlval (lval, 0);
      freelval (lval);
      lval[LTYPE] = BRANCH;
      lval[LVALUE] = _EQ;
      lval[LFALSE] = lval[LTRUE] = 0;
    }

    if (hier_oper[entry] == _LAND) {
      if (lval[LFALSE])
        fprintf (outhdl, "_%d=_%d\n", lval[LFALSE], lbl);
      lval[LFALSE] = lbl;
    } else {
      if (lval[LTRUE])
        fprintf (outhdl, "_%d=_%d\n", lval[LTRUE], lbl);
      lval[LTRUE] = lbl;
    }
  }
}

/*
 * Auto increment/decrement
 */
step (pre, lval, post)
register int pre, lval[], post;
{
int dest[LLAST];
register int reg;

  if ((lval[LTYPE] == EXPR) || (lval[LTYPE] == CONSTANT) || (lval[LTYPE] == BRANCH))
    error ("non-modifiable variable");

  // Copy lval
  dest[LTYPE]  = lval[LTYPE];
  dest[LPTR]   = lval[LPTR];
  dest[LSIZE]  = lval[LSIZE];
  dest[LEA]    = lval[LEA];
  dest[LNAME]  = lval[LNAME];
  dest[LVALUE] = lval[LVALUE];
  dest[LREG1]  = lval[LREG1];
  dest[LREG2]  = lval[LREG2];

  if (lval[LEA] == EA_REG) {
    gencode_R ((pre|post), lval[LREG1], lval_ISIPTR ? REG_BPW : REG_1);
    if (post) {
      reg = allocreg ();
      gencode_R (_LODR, reg, lval[LREG1]);
      gencode_R ((_ADD+_SUB-post), reg, lval_ISIPTR ? REG_BPW : REG_1);
      freelval (lval);
      lval[LREG1] = reg;
    }
  } else {
    reg = allocreg ();
    loadlval (lval, reg);
    gencode_R ((pre|post), lval[LREG1], lval_ISIPTR ? REG_BPW : REG_1);
    gencode_M (dest_ISBPW ? _STOW : _STOB, lval[LREG1], dest);
    if (post) {
      gencode_R ((_ADD+_SUB-post), reg, lval_ISIPTR ? REG_BPW : REG_1);
      lval[LREG1] = reg;
    }
  }
}

/*
 * Load primary expression
 */
primary (lval)
register int lval[];
{
register int *ident, i;
int sname, len;

  if (match ("(")) {  // (expression,...)
    expression (lval, 1);
    needtoken (")");
    return 1;
  }

  // load identifier
  if (!(len = dohash (lptr, &sname)))
    return constant (lval);
  bump(len); // Skip identifier

  // identifier
  for (i=idinx-1; i>=0; i--) {
    ident = &idents[i*ILAST];
    if (ident[INAME] == sname) {
      lval[LTYPE] = ident[ITYPE];
      lval[LPTR] = ident[IPTR];
      lval[LSIZE] = ident[ISIZE];
      lval[LNAME] = lval[LVALUE] = lval[LREG1] = lval[LREG2] = 0;

      if (ident[ICLASS] == REGISTER) {
        lval[LREG1] = ident[IVALUE];
        lval[LEA] = EA_REG;
      } else if (ident[ICLASS] == AP_AUTO) {
        lval[LREG1] = REG_AP;
        lval[LVALUE] = ident[IVALUE];
        lval[LEA] = EA_IND;
      } else if (ident[ICLASS] == SP_AUTO) {
        lval[LREG1] = REG_SP;
        lval[LVALUE] = ident[IVALUE];
        lval[LEA] = EA_IND;
      } else if (ident[ITYPE] == FUNCTION && !ident[IPTR]) {
        lval[LNAME] = sname;
        lval[LEA] = EA_ADDR;
      } else {
	lval[LNAME] = sname;
        lval[LEA] = EA_IND;
      }

      // Convert arrays into pointers
      if (ident[ITYPE] == ARRAY) {
        lval[LTYPE] = VARIABLE;
        lval[LEA] = EA_ADDR;
      }
      return 1;
    }
  }

  // test for reserved words
  if (sname == argcid) {
    // generate (2(AP)-BPW)/BPW
    lval[LTYPE] = EXPR;
    lval[LPTR] = 0;
    lval[LSIZE] = BPW;
    lval[LEA] = EA_REG;
    lval[LNAME] = lval[LVALUE] = 0;
    lval[LREG1] = allocreg ();
    lval[LREG2] = 0;

    gencode_IND (_LODW, lval[LREG1], BPW, REG_AP);
    gencode_R (_SUB, lval[LREG1], REG_BPW);
    gencode_R (_DIV, lval[LREG1], REG_BPW);
    return 1;
  } else if (sname == argvid) {
    exprerr ();
    return 0;
  }

  // make it AUTOEXT
  lval[LTYPE] = FUNCTION;
  lval[LPTR] = 0;
  lval[LSIZE] = BPW;
  lval[LEA] = EA_ADDR;
  lval[LNAME] = sname;
  lval[LREG1] = lval[LREG2] = lval[LVALUE] = 0;

  // add symbol to symboltable
  if (idinx >= IDMAX)
    fatal ("identifier table overflow");
  ident = &idents[idinx++ * ILAST];
  ident[INAME] = sname;
  ident[ITYPE] = FUNCTION;
  ident[IPTR] = 0;
  ident[ICLASS] = AUTOEXT;
  ident[IVALUE] = 0;
  ident[ISIZE] = BPW;

  return 1;
}

/*
 * Do a hierarchical evaluation
 */
hier14 (lval)
register int lval[];
{
int lval2[LLAST], sav_csp;
register int argc, reg;

  if (!primary (lval))
    return 0;
  if (match("[")) { // [subscript]
    if (!lval[LPTR])
      error ("can't subscript");
    else if (!hier1 (lval2))
      error ("need subscript");
    else {
      if (lval2[LTYPE] == CONSTANT) {
        if (lval[LEA] == EA_IND)
          loadlval (lval, 0); // make LVALUE available
        // Subscript is a constant
        lval[LVALUE] += lval2[LVALUE] * lval[LSIZE];
      } else {
        // Subscript is a variable/complex-expression
        if ((lval[LEA] == EA_IND) || lval[LREG2])
          loadlval (lval, 0); // make LREG2 available
        loadlval (lval2, 0);
        if (lval[LSIZE] == BPW)
          gencode_R (_MUL, lval2[LREG1], REG_BPW); // size index
        if (!lval[LREG1])
          lval[LREG1] = lval2[LREG1];
        else
          lval[LREG2] = lval2[LREG1];
      }
      // Update data type
      lval[LPTR] = 0;
      lval[LEA] = EA_IND;
    }
    needtoken ("]");
  }
  if (match ("(")) { // function (...)
    if (lval[LPTR] || (lval[LTYPE] != FUNCTION))
      error ("Illegal function");

    argc = BPW;
    sav_csp = csp;
    blanks ();
    while (ch != ')') {
      // Get expression
      expression(lval2, 0);
      if (lval2[LTYPE] == CONSTANT) {
        gencode_I (_PSHA, 0, lval2[LVALUE]);
      } else {
        if (lval2[LTYPE] == BRANCH)
          loadlval (lval2, 0);
        freelval (lval2);
        // Push onto stack
        if (lval2[LEA] != EA_IND)
          gencode_M (_PSHA, 0, lval2);
        else
          gencode_M (lval2_ISBPW ? _PSHW : _PSHB, 0, lval2);
      }
      // increment ARGC
      csp -= BPW;
      argc += BPW;

      if (!match (","))
        break;
    }
    needtoken (")");
    // Push ARGC
    if (argc == BPW)
      reg = REG_BPW;
    else if (argc == 4)
      reg = REG_4;
    else {
      reg = allocreg ();
      gencode_I (_LEA, reg, argc);
    }
    gencode_IND (_PSHA, 0, 0, reg);
    // call
    gencode_M (_JSB, 0, lval);
    freelval (lval);
    // Pop args
    gencode_R (_ADD, REG_SP, reg);
    if (reg < REG_0)
      freereg (reg);
    csp = sav_csp;

    lval[LTYPE] = EXPR;
    lval[LPTR] = 0;
    lval[LSIZE] = BPW;
    lval[LEA] = EA_REG;
    lval[LNAME] = lval[LVALUE] = 0;
    lval[LREG1] = 1;
    lval[LREG2] = 0;
  }
  return 1;
}

hier13 (lval)
register int lval[];
{

  if (match ("++")) {
    if (!hier13 (lval)) {
      exprerr ();
      return 0;
    }
    step (_ADD, lval, 0);
  } else if (match ("--")) {
    if (!hier13 (lval)) {
      exprerr ();
      return 0;
    }
    step (_SUB, lval, 0);
  } else if (match ("~")) {
    if (!hier13 (lval)) {
      exprerr ();
      return 0;
    }
    if (lval[LTYPE] == CONSTANT)
      lval[LVALUE] = ~lval[LVALUE];
    else {
      loadlval (lval, 0);
      gencode_R (_NOT, 0, lval[LREG1]);
    }
  } else if (match ("!")) {
    if (!hier13 (lval)) {
      exprerr ();
      return 0;
    }
    if (lval[LTYPE] == CONSTANT)
      lval[LVALUE] = !lval[LVALUE];
    else if (lval[LTYPE] == BRANCH)
      lval[LVALUE] = negop(lval[LVALUE]);
    else {
      // convert CC bits into a BRANCH
      loadlval (lval, 0);
      freelval (lval);
      lval[LTYPE] = BRANCH;
      lval[LVALUE] = _NE;
      lval[LFALSE] = lval[LTRUE] = 0;
    }
  } else if (match ("-")) {
    if (!hier13 (lval)) {
      exprerr ();
      return 0;
    }
    if (lval[LTYPE] == CONSTANT)
      lval[LVALUE] = -lval[LVALUE];
    else {
      loadlval (lval, 0);
      gencode_R (_NEG, 0, lval[LREG1]);
    }
  } else if (match ("+")) {
    if (!hier13 (lval)) {
      exprerr ();
      return 0;
    }
  } else if (match ("*")) {
    if (!hier13 (lval)) {
      exprerr ();
      return 0;
    }
    if (!lval[LPTR] || (lval[LTYPE] == CONSTANT) || (lval[LTYPE] == BRANCH))
      error ("Illegal address");
    else {
      if (lval[LEA] == EA_IND)
        loadlval (lval, 0);
      lval[LPTR] = 0;
      lval[LEA] = EA_IND;
    }
  } else if (match ("&")) {
    if (!hier13 (lval)) {
      exprerr ();
      return 0;
    }
    if ((lval[LEA] != EA_IND) || (lval[LTYPE] == CONSTANT) || (lval[LTYPE] == BRANCH))
      error ("Illegal address");
    else {
      lval[LTYPE] = EXPR;
      lval[LSIZE] = lval_ISBPW ? BPW : 1;
      lval[LPTR] = 1;
      lval[LEA] = EA_ADDR;
    }
  } else {
    if (!hier14 (lval))
      return 0;
    if (match ("++")) {
      step (0, lval, _ADD);
    } else if (match ("--")) {
      step (0, lval, _SUB);
    }
  }
  return 1;
}

hier12 (lval)
int lval[];
{
  return xplng1 (hier13, 24, lval);
}

hier11 (lval)
int lval[];
{
  return xplng1 (hier12, 21, lval);
}

hier10 (lval)
int lval[];
{
  return xplng1 (hier11, 18, lval);
}

hier9 (lval)
int lval[];
{
  return xplng2 (hier10, 13, lval);
}

hier8 (lval)
int lval[];
{
  return xplng2 (hier9, 10, lval);
}

hier7 (lval)
int lval[];
{
  return xplng1 (hier8, 8, lval);
}

hier6 (lval)
int lval[];
{
  return xplng1 (hier7, 6, lval);
}

hier5 (lval)
int lval[];
{
  return xplng1 (hier6, 4, lval);
}

hier4 (lval)
int lval[];
{
  return xplng3 (hier5, 2, lval);
}

hier3 (lval)
int lval[];
{
  return xplng3 (hier4, 0, lval);
}

hier2 (lval)
register int lval[];
{
register int lbl, reg;

  if (!hier3(lval))
    return 0;
  if (!match ("?"))
    return 1;

  // If not a BRANCH then convert it into one
  if (lval[LTYPE] != BRANCH) {
    loadlval (lval, 0);
    freelval (lval);
    lval[LTYPE] = BRANCH;
    lval[LVALUE] = _EQ;
    lval[LFALSE] = lval[LTRUE] = 0;
  }
  // alloc labels
  if (!lval[LFALSE])
    lval[LFALSE] = ++nxtlabel;

  // process 'true' variant
  gencode_L (lval[LVALUE], lval[LFALSE]);
  if (lval[LTRUE])
    fprintf (outhdl, "_%d:", lval[LTRUE]);
  expression(lval, 1);
  loadlval (lval, reg=allocreg()); // Needed to assign a dest reg

  needtoken (":");
  // jump to end
  lbl = ++nxtlabel;
  gencode_L (_JMP, lbl);

  // process 'false' variant
  fprintf (outhdl, "_%d:", lval[LFALSE]);
  if (!hier1 (lval))
    exprerr ();
  else
    loadlval (lval, reg); // Needed for result to occupy same reg

  fprintf (outhdl, "_%d:", lbl);

  // resulting type is undefined, so modify LTYPE
  lval[LTYPE] = EXPR;
  lval[LPTR] = 0;
  lval[LEA] = EA_REG;

  return 1;
}

hier1 (lval)
register int lval[];
{
int rval[LLAST], dest[LLAST];
register int oper;

  if (!hier2(lval))
    return 0;

  // Test for assignment
       if (omatch ("="))  oper = -1;
  else if (match ("|="))  oper = _BOR;
  else if (match ("^="))  oper = _XOR;
  else if (match ("&="))  oper = _BAND;
  else if (match ("+="))  oper = _ADD;
  else if (match ("-="))  oper = _SUB;
  else if (match ("*="))  oper = _MUL;
  else if (match ("/="))  oper = _DIV;
  else if (match ("%="))  oper = _MOD;
  else if (match (">>=")) oper = _LSR;
  else if (match ("<<=")) oper = _LSL;
  else
    return 1;

  // test if lval modifiable
  if ((lval[LTYPE] == EXPR) || (lval[LTYPE] == CONSTANT) || (lval[LTYPE] == BRANCH))
    error ("Inproper lvalue");

  // Get rval
  if (!hier1 (rval)) {
    exprerr ();
    return 1;
  }
  loadlval (rval, -1);

  if (oper == -1) {
    if (lval[LEA] == EA_REG)
      gencode_R (_LODR, lval[LREG1], rval[LREG1]);
    else {
      gencode_M (lval_ISBPW ? _STOW : _STOB, rval[LREG1], lval);
      freelval (lval);
    }
    lval[LNAME] = lval[LVALUE] = 0;
    lval[LREG1] = rval[LREG1];
    lval[LREG2] = 0;
  } else {
    // Copy lval
    dest[LTYPE]  = lval[LTYPE];
    dest[LPTR]   = lval[LPTR];
    dest[LSIZE]  = lval[LSIZE];
    dest[LEA]    = lval[LEA];
    dest[LNAME]  = lval[LNAME];
    dest[LVALUE] = lval[LVALUE];
    dest[LREG1]  = lval[LREG1];
    dest[LREG2]  = lval[LREG2];

    // load lval into reg, modify it with rval and copy result into dest
    loadlval (lval, allocreg ()); // don't reuse regs for dest
    gencode_R (oper, lval[LREG1], rval[LREG1]);
    freelval (rval);
    if (dest[LEA] == EA_REG)
      gencode_R (_LODR, dest[LREG1], lval[LREG1]);
    else
      gencode_M (lval_ISBPW ? _STOW : _STOB, lval[LREG1], dest);
  }  

  // resulting type is undefined, so modify LTYPE
  lval[LTYPE] = EXPR;
  lval[LPTR] = 0;
  lval[LEA] = EA_REG;

  return 1;
}

/*
 * Load a numerical expression seperated by comma's
 */
expression(lval, comma)
register int lval[], comma;
{
  lval[LTYPE] = CONSTANT;
  do {
    if (lval[LTYPE] != CONSTANT)
      freelval (lval);
    if (!hier1 (lval))
      error ("expression required");
  } while (comma && match (","));
}

/* 
 * Load a constant expression
 */
constexpr (val)
register int *val;
{
int lval[LLAST];

  if (!hier1 (lval))
    return 0;
  if (lval[LTYPE] == CONSTANT) {
    *val = lval[LVALUE];
    return 1;
  }
  error ("must be a constant expression");
  freelval (lval);
  return 0;
}


/*
 * Load a constant value
 */
constant (lval)
register int lval[];
{
  lval[LTYPE] = CONSTANT;
  if (number(&lval[LVALUE]))
    return 1;
  if (pstr(&lval[LVALUE]))
    return 1;
  if (qstr()) {
    // Convert to char pointer
    lval[LTYPE] = LABEL;
    lval[LPTR] = 1;
    lval[LSIZE] = 1; 
    lval[LEA] = EA_ADDR;
    lval[LNAME] = ++nxtlabel;
    lval[LVALUE] = lval[LREG1] = lval[LREG2] = 0;
    // Generate data
    toseg (DATASEG);
    fprintf (outhdl, "_%d:", lval[LNAME]);
    dumplits (1);
    toseg (prevseg);
    return 1;
  }
  return 0;
}

/*
 * Get a numerical constant
 */
number (val)
register int *val;
{
register int i, minus;

  i = minus = 0;
  if (!isdigit (ch)) 
    return 0;
  if ((ch ==  '0') && (toupper (nch) == 'X')) {
    bump (2);
    while (isxdigit (ch)) {
      if (ch <= '9')
        i = i * 16 + (gch () - '0');
      else if (ch >= 'a')
        i = i * 16 + (gch () - 'a' + 10);
      else
        i = i * 16 + (gch () - 'A' + 10);
    }
  } else {
    while (isdigit (ch))
      i = i * 10 + (gch () - '0');
  }
  *val = i;
  return 1;
}

/*
 * Get a character constant
 */
pstr (val)
int *val;
{
register int i;

  i = 0;
  if (!match ("'"))
    return 0;
  while (ch && (ch != '\''))
    i = (i<<8) + litchar();
  gch ();
  *val = i;
  return 1;
}

/* 
 * Get a string constant
 */
qstr ()
{
  if (!match ("\""))
    return 0;

  litinx = 0;
  while (ch && (ch != '"'))
    addlits (litchar (), 1);
  gch (); // skip terminator
  addlits (0, 1);
  return 1;
}

/*
 * Return current literal character and bump lptr
 */
litchar ()
{
register int i,oct;

  if ((ch != '\\') || (nch == 0))
    return gch ();
  gch ();
  switch (ch) {
    case 'n':
      gch ();
      return NEWLINE;
    case 't':
      gch ();
      return HT;
    case 'b':
      gch ();
      return BS;
    case 'f':
      gch ();
      return FF;
  }
  i = 0;
  oct = 0;
  while ((ch >= '0') && (ch <= '7')) {
    oct = (oct<<3) + gch() - '0';
    ++i;
  }
  return i ? oct : gch ();
}

/*
 * Add a value to the literal pool
 */
addlits (val, size)
register int val, size;
{
  if (size == BPW) {
    litq[litinx++] = val>>8;
    if (litinx >= LITMAX)
      fatal ("Literal queue overflow");
  }
  litq[litinx++] = val;
  if (litinx >= LITMAX)
    fatal ("Literal queue overflow");
}

/*
 * dump the literal pool
 */
dumplits(size)
int size;
{
register int i, j;

  if (!litinx)
    return;

  i = 0;
  while (i < litinx) {
    if (size == 1)
      fprintf (outhdl, "\t.DCB ");
    else
      fprintf (outhdl, "\t.DCW ");

    for (j=0; j<16; j++) {
    	if (size == 1)
	  fprintf (outhdl, "%d", (i >= litinx) ? 0 : litq[i]);
    	else
          fprintf (outhdl, "%d", (i >= litinx) ? 0 : ((litq[i+0]<<8) | (litq[i+1] & 0xff)));
      i += size;
      if (i >= litinx)
        break;
      if (j != 15)
        fprintf (outhdl, ",");
    }
    fprintf (outhdl, "\n");
  }
  litinx = 0;
}
