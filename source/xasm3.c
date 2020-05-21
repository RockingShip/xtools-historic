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
//* X-Assembler.  Part 3, Expression evaluation
//*

#define EXTERN extern
#include "xasm.h"

/*
 * Process a constant evaluation
 */
calc(register int left, register int right, int oper) {
	switch (oper) {
	case REL_OR :  return (left  |  right);
	case REL_XOR:  return (left  ^  right);
	case REL_AND:  return (left  &  right);
	case REL_LSR:  return (left  >> right);
	case REL_LSL:  return (left  << right);
	case REL_ADD:  return (left  +  right);
	case REL_SUB:  return (left  -  right);
	case REL_MUL:  return (left  *  right);
	case REL_DIV:  return (left  /  right);
	case REL_MOD:  return (left  %  right);
	default:     return 0;
	}
}

loadlval(register int lval[]) {
	register int *p;

	if (lval[LTYPE] == CONSTANT) {
		sto_cmd(REL_PUSHW, lval[LVALUE]);
		lval[LTYPE] = EXPRESSION;
	} else if (lval[LTYPE] == SYMBOL) {
		p = &name[lval[LVALUE] * NLAST];
		switch (p[NTYPE]) {
		case ABS:
			sto_cmd(REL_PUSHW, p[NVALUE]);
			break;
		case CODE:
			sto_cmd(REL_CODEW, p[NVALUE]);
			break;
		case DATA:
			sto_cmd(REL_DATAW, p[NVALUE]);
			break;
		case UDEF:
			sto_cmd(REL_UDEFW, p[NVALUE]);
			break;
		default:
			error("Symbol not proper type");
			break;
		}
		lval[LTYPE] = EXPRESSION;
	}
}

/*
 * generic processing for <lval> { <operation> <rval> }
 */
xplng1(register int (*hier)(), register int start, register int lval[]) {
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
			if (omatch(cptr))
				break;
			++entry;
		}

		// Load rval
		if (!(*hier)(rval)) {
			exprerr();
			return 1;
		}

		// Generate code
		if ((lval[LTYPE] == CONSTANT) && (rval[LTYPE] == CONSTANT)) {
			lval[LVALUE] = calc(lval[LVALUE], hier_oper[entry], rval[LVALUE]);
		} else {
			if ((lval[LTYPE] != EXPRESSION) && (rval[LTYPE] == EXPRESSION)) {
				loadlval(lval);
				sto_cmd(REL_SWAP, 0);
			} else if ((lval[LTYPE] == EXPRESSION) && (rval[LTYPE] != EXPRESSION)) {
				loadlval(rval);
			} else if ((lval[LTYPE] != EXPRESSION) && (rval[LTYPE] != EXPRESSION)) {
				loadlval(lval);
				loadlval(rval);
			}
			sto_cmd(hier_oper[entry], 0);
			lval[LTYPE] = EXPRESSION;
		}
	}
}

primary(register int lval[]) {
	register int *p, len;
	int hash;

	if (match("<")) {  // <expression,...>
		expression(lval);
		needtoken(">");
		return 1;
	}

	// test for identifier
	len = dohash(lptr, &hash);
	if (len) {
		bump(len);
		while (1) {
			p = &name[hash * NLAST];
			if (p[NTYPE] != LINK)
				break;
			else
				hash = p[NVALUE];
		}
		if (!p[NTYPE])
			p[NTYPE] = UNDEF; // Initial value

		switch (p[NTYPE]) {
		case UNDEF:
			sto_cmd(REL_SYMBOL, hash);
			lval[LTYPE] = EXPRESSION;
			break;
		case ABS:
		case CODE:
		case DATA:
		case UDEF:
			lval[LTYPE] = SYMBOL;
			lval[LVALUE] = hash;
			break;
		case POINT:
			switch (curseg) {
			case CODESEG:
				sto_cmd(REL_CODEW, curpos[CODESEG]);
				break;
			case DATASEG:
				sto_cmd(REL_DATAW, curpos[DATASEG]);
				break;
			case UDEFSEG:
				sto_cmd(REL_UDEFW, curpos[UDEFSEG]);
				break;
			}
			lval[LTYPE] = EXPRESSION;
			break;
		default:
			error("Invalid expression");
			lval[LTYPE] = CONSTANT;
			break;
		}
		return 1;
	}

	// test for constant
	return constant(lval);
}

hier7(register int lval[]) {

	if (match("~")) {
		if (!hier7(lval)) {
			exprerr();
			return 0;
		}
		if (lval[LTYPE] == CONSTANT)
			lval[LVALUE] = ~lval[LVALUE];
		else {
			if (lval[LTYPE] != EXPRESSION)
				loadlval(lval);
			sto_cmd(REL_NOT, 0);
		}
	} else if (match("-")) {
		if (!hier7(lval)) {
			exprerr();
			return 0;
		}
		if (lval[LTYPE] == CONSTANT)
			lval[LVALUE] = -lval[LVALUE];
		else {
			if (lval[LTYPE] != EXPRESSION)
				loadlval(lval);
			sto_cmd(REL_NEG, 0);
		}
	} else if (match("+")) {
		if (!hier7(lval)) {
			exprerr();
			return 0;
		}
	} else if (!primary(lval))
		return 0;
	return 1;
}

hier6(register int lval[]) {
	xplng1(hier7, 12, lval);
}

hier5(register int lval[]) {
	xplng1(hier6, 9, lval);
}

hier4(register int lval[]) {
	xplng1(hier5, 6, lval);
}

hier3(register int lval[]) {
	xplng1(hier4, 4, lval);
}

hier2(register int lval[]) {
	xplng1(hier3, 2, lval);
}

hier1(register int lval[]) {
	xplng1(hier2, 0, lval);
}

/*
 * Load a numerical expression seperated by comma's
 */
expression(register int lval[]) {
	if (!hier1(lval))
		error("expression required");
}

/*
 * Load a constant expression
 */
constexpr(register int *val) {
	int lval[LLAST];

	if (!hier1(lval))
		return 0;
	if (lval[LTYPE] == CONSTANT) {
		*val = lval[LVALUE];
		return 1;
	}
	return 0;
}

/*
 * Load a constant value
 */
constant(register int lval[]) {
	lval[LTYPE] = CONSTANT;
	if (number(&lval[LVALUE]))
		return 1;
	if (pstr(&lval[LVALUE]))
		return 1;
	return 0;
}

/*
 * Get a numerical constant
 */
number(register int *val) {
	register int i, minus;

	i = minus = 0;
	if (!isdigit(ch))
		return 0;
	if ((ch == '0') && (toupper(nch) == 'X')) {
		bump(2);
		while (isxdigit(ch)) {
			if (ch <= '9')
				i = i * 16 + (gch() - '0');
			else if (ch >= 'a')
				i = i * 16 + (gch() - 'a' + 10);
			else
				i = i * 16 + (gch() - 'A' + 10);
		}
	} else {
		while (isdigit(ch))
			i = i * 10 + (gch() - '0');
	}
	*val = i;
	return 1;
}

/*
 * Get a character constant
 */
pstr(int *val) {
	register int i;

	i = 0;
	if (!match("'"))
		return 0;
	while (ch && (ch != '\''))
		i = (i << 8) + litchar();
	gch();
	*val = i;
	return 1;
}

/*
 * Return current literal character and bump lptr
 */
litchar() {
	register int i, oct;

	if ((ch != '\\') || (nch == 0))
		return gch();
	gch();
	switch (ch) {
	case 'b': // bell
		gch();
		return 8;
	case 'f': // form-feed
		gch();
		return 12;
	case 'n': // newline
		gch();
		return 10;
	case 'r': // carriage return
		gch();
		return 13;
	case 't': // horizontal tab
		gch();
		return 9;
	case 'v': // vertical tab
		gch();
		return 11;
	}
	i = 0;
	oct = 0;
	while ((ch >= '0') && (ch <= '7')) {
		oct = (oct << 3) + gch() - '0';
		++i;
	}
	return i ? oct : gch();
}
