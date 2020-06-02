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
//* X-Assembler.  Part 2, Syntax control
//*

#define EXTERN extern
#include "xasm.h"

need_comma() {
	blanks();
	if (ch == ',')
		gch();
	else
		error("comma expected");
}

need_reg() {
	int hash, reg;
	register int len, *p;

	blanks();
	if (!ch)
		len = 0;
	else
		len = dohash(lptr, &hash);
	if (!len) {
		error("register expected");
		reg = 0;
	} else {
		bump(len);
		while (1) {
			p = &names[hash * NLAST];
			if (p[NTYPE] != LINK)
				break;
			else
				hash = p[NVALUE];
		}
		if (p[NTYPE] != REGISTER) {
			error("register expected");
			reg = 0;
		} else
			reg = p[NVALUE];
	}
	// process reg
	sto_data(reg, 1);
}

need_imm() {
	int lval[LLAST];

	expression(lval);
	if (lval[LTYPE] == CONSTANT)
		sto_data(lval[LVALUE], BPW);
	else {
		loadlval(lval);
		sto_cmd(REL_POPW, 0);
	}
}

need_mem() {
	int lval[LLAST];

	blanks();
	if (ch != '(')
		need_imm();
	else
		sto_data(0, BPW); // no address

	// test for registers
	blanks();
	if (ch != '(') {
		sto_data(0, 1); // dummy reg
	} else {
		gch();
		blanks();
		need_reg();
		blanks();
		if (ch == ')')
			gch();
		else
			error(") expected");
	}
}

do_opcode(register int p[]) {
	if (pass == 1) {
		switch (p[NVALUE]) {
			case OPC_ILLEGAL:
			case OPC_RSB:
				curpos[curseg] += 1;
				break;
			case OPC_PSHR:
			case OPC_POPR:
			case OPC_SVC:
				curpos[curseg] += 3;
				break;
			case OPC_PSHB:
			case OPC_PSHW:
			case OPC_PSHA:
			case OPC_JSB:
				curpos[curseg] += 4;
				break;
			case OPC_NEG:
			case OPC_NOT:
				curpos[curseg] += 2;
				break;
			case OPC_ADD:
			case OPC_SUB:
			case OPC_MUL:
			case OPC_DIV:
			case OPC_MOD:
			case OPC_OR:
			case OPC_XOR:
			case OPC_AND:
			case OPC_LSR:
			case OPC_LSL:
			case OPC_LDR:
			case OPC_CMP:
				curpos[curseg] += 3;
				break;
			case OPC_BEQ:
			case OPC_BNE:
			case OPC_BLT:
			case OPC_BLE:
			case OPC_BGT:
			case OPC_BGE:
			case OPC_JMP:
				curpos[curseg] += 4;
				break;
			case OPC_LDB:
			case OPC_LDW:
			case OPC_LEA:
			case OPC_STB:
			case OPC_STW:
				curpos[curseg] += 5;
				break;
			default:
				error("unimplemented opcode");
				break;
		}
		ch = 0; // ignore rest of line
	} else {
		sto_data(p[NVALUE], 1);
		switch (p[NVALUE]) {
		case OPC_ILLEGAL:
		case OPC_RSB:
			curpos[curseg] += 1;
			break;
			break;
		case OPC_PSHR:
		case OPC_POPR:
		case OPC_SVC:
			need_imm();
			curpos[curseg] += 3;
			break;
		case OPC_PSHB:
		case OPC_PSHW:
		case OPC_PSHA:
		case OPC_JSB:
			need_mem();
			curpos[curseg] += 4;
			break;
		case OPC_NEG:
		case OPC_NOT:
			need_reg();
			curpos[curseg] += 2;
			break;
		case OPC_ADD:
		case OPC_SUB:
		case OPC_MUL:
		case OPC_DIV:
		case OPC_MOD:
		case OPC_OR:
		case OPC_XOR:
		case OPC_AND:
		case OPC_LSR:
		case OPC_LSL:
		case OPC_LDR:
		case OPC_CMP:
			need_reg();
			need_comma();
			need_reg();
			curpos[curseg] += 3;
			break;
		case OPC_BEQ:
		case OPC_BNE:
		case OPC_BLT:
		case OPC_BLE:
		case OPC_BGT:
		case OPC_BGE:
		case OPC_JMP:
			need_mem();
			curpos[curseg] += 4;
			break;
		case OPC_LDB:
		case OPC_LDW:
		case OPC_LEA:
		case OPC_STB:
		case OPC_STW:
			need_reg();
			need_comma();
			need_mem();
			curpos[curseg] += 5;
			break;
		default:
			error("unimplemented opcode");
			ch = 0; // ignore rest of line
			break;
		}
	}
}

/*
 * Unsigned compare GT
 * Trying all possibilities reveals:
 * unsigned "i>j" can be rewritten as "(j^i)&0x8000 ?  i&0x8000 : (j-i)&0x8000"
 */
unsigned_GT(int i, int j) {
	return ((j ^ i) & (1 << SBIT) ? i & (1 << SBIT) : (j - i) & (1 << SBIT));
}

save_seg_size() {
	if (unsigned_GT(curpos[CODESEG], maxpos[CODESEG]))
		maxpos[CODESEG] = curpos[CODESEG];
	if (unsigned_GT(curpos[DATASEG], maxpos[DATASEG]))
		maxpos[DATASEG] = curpos[DATASEG];
	if (unsigned_GT(curpos[TEXTSEG], maxpos[TEXTSEG]))
		maxpos[TEXTSEG] = curpos[TEXTSEG];
	if (unsigned_GT(curpos[UDEFSEG], maxpos[UDEFSEG]))
		maxpos[UDEFSEG] = curpos[UDEFSEG];
}

do_pseudo(register int p[]) {
	int val, lval[LLAST];
	register int size;

	switch (p[NVALUE]) {
	case PSEUDO_CODE:
		curseg = CODESEG;
		sto_cmd(REL_CODEORG, curpos[CODESEG]);
		break;
	case PSEUDO_DATA:
		curseg = DATASEG;
		sto_cmd(REL_DATAORG, curpos[DATASEG]);
		break;
	case PSEUDO_TEXT:
		curseg = TEXTSEG;
		sto_cmd(REL_TEXTORG, curpos[TEXTSEG]);
		break;
	case PSEUDO_UDEF:
		curseg = UDEFSEG;
		sto_cmd(REL_UDEFORG, curpos[UDEFSEG]);
		break;
	case PSEUDO_DSB:
	case PSEUDO_DSW:
		size = p[NVALUE] == PSEUDO_DSB ? 1 : BPW;
		if (!constexpr(&val)) {
			if (pass == 1)
				error("constant required");
		} else
			curpos[curseg] += val * size;
		sto_cmd(REL_DSB, val * size);
		break;
	case PSEUDO_DCB:
	case PSEUDO_DCW:
		size = p[NVALUE] == PSEUDO_DCB ? 1 : BPW;
		while (1) {
			if (match("\"")) {
				while (ch && (ch != '"')) {
					val = litchar();
					if (pass == 2)
						sto_data(val, size);
					curpos[curseg] += size;
				}
				gch(); // skip terminator
			} else {
				// get an expression
				if (!expr_or(lval))
					break;

				if (pass == 2) {
					if (lval[LTYPE] != CONSTANT) {
						loadlval(lval);
						if (size == 1)
							sto_cmd(REL_POPB, 0);
						else
							sto_cmd(REL_POPW, 0);
					} else if (size == BPW)
						sto_data(lval[LVALUE], BPW);
					else if ((lval[LVALUE] >= -128) && (lval[LVALUE] <= 127))
						sto_data(lval[LVALUE], 1);
					else
						error("constant out of range");
				}
				curpos[curseg] += size;
			}
			if (!match(","))
				break;
		}
		break;
	case PSEUDO_ORG:
		save_seg_size();
		expression(lval);
		if (lval[LTYPE] == CONSTANT) {
			curpos[curseg] = lval[LVALUE];
		} else if (lval[LTYPE] == SYMBOL) {
			p = &names[lval[LVALUE] * NLAST];
			switch (p[NTYPE]) {
			case CODE:
				curseg = CODESEG;
				break;
			case DATA:
				curseg = DATASEG;
				break;
			case TEXT:
				curseg = TEXTSEG;
				break;
			case UDEF:
				curseg = UDEFSEG;
				break;
			}
			curpos[curseg] = p[NVALUE];
		} else
			error("Invalid address");
		if (pass == 2) {
			switch (curseg) {
			case CODESEG:
				sto_cmd(REL_CODEORG, curpos[curseg]);
				break;
			case DATASEG:
				sto_cmd(REL_DATAORG, curpos[curseg]);
				break;
			case TEXTSEG:
				sto_cmd(REL_TEXTORG, curpos[curseg]);
				break;
			case UDEFSEG:
				sto_cmd(REL_UDEFORG, curpos[curseg]);
				break;
			}
		}
		break;
	default:
		error("-> pseudo");
		ch = 0;
		break;
	}
}

parse() {
	int i, len, hash, ext;
	register int *p;
	int lval[LLAST];

	while (inphdl) {
		if (amatch("#include")) {
			doinclude();
			continue;
		}
		if (amatch("#define")) {
			declmac();
			continue;
		}

		if (debug && (pass == 2))
			fprintf(outhdl, ";%s\n", lptr);
		while (1) {
			blanks();
			if (!ch)
				break; // end of line
			len = dohash(lptr, &hash);
			if (!len) {
				if (pass == 1)
					error("opcode expected");
				ch = 0;
				break;
			}
			bump(len);
			p = &names[hash * NLAST];
			if (!p[NTYPE])
				p[NTYPE] = UNDEF;

			switch (p[NTYPE]) {
			case UNDEF:
				if (match(":")) {
					ext = match(":");
					if (verbose && ext)
						printf("%s\n", line);
					if (curseg == CODESEG) {
						p[NTYPE] = CODE;
						p[NVALUE] = curpos[CODESEG];
					} else if (curseg == DATASEG) {
						p[NTYPE] = DATA;
						p[NVALUE] = curpos[DATASEG];
					} else if (curseg == TEXTSEG) {
						p[NTYPE] = TEXT;
						p[NVALUE] = curpos[TEXTSEG];
					} else if (curseg == UDEFSEG) {
						p[NTYPE] = UDEF;
						p[NVALUE] = curpos[UDEFSEG];
					} else
						error("unknown segment");
					continue;
				} else if (match("=")) {
					len = dohash(lptr, &p[NVALUE]);
					if (!len) {
						if (pass == 1)
							error("use #define");
						ch = 0;
					} else {
						bump(len);
						p[NTYPE] = LINK;
						p = &names[p[NVALUE] * NLAST];
						if (!p[NTYPE])
							p[NTYPE] = UNDEF; // Initial value
					}
					break;
				} else {
					if (pass == 1)
						error("unknown opcode");
					ch = 0;
					break;
				}
			case ABS:
			case CODE:
			case DATA:
			case TEXT:
			case UDEF:
				if (match(":")) {
					ext = match(":");
					if (verbose && ext)
						printf("%s\n", line);
					if (p[NTYPE] == CODE) {
						if ((curseg != CODESEG) || (p[NVALUE] != curpos[CODESEG]))
							if (pass == 1)
								error("multiply defined");
							else
								error("phase error");
						if (ext && (pass == 2))
							sto_cmd(REL_CODEDEF, hash);
					} else if (p[NTYPE] == DATA) {
						if ((curseg != DATASEG) || (p[NVALUE] != curpos[DATASEG]))
							if (pass == 1)
								error("multiply defined");
							else
								error("phase error");
						if (ext && (pass == 2))
							sto_cmd(REL_DATADEF, hash);
					} else if (p[NTYPE] == TEXT) {
						if ((curseg != TEXTSEG) || (p[NVALUE] != curpos[TEXTSEG]))
							if (pass == 1)
								error("multiply defined");
							else
								error("phase error");
						if (ext && (pass == 2))
							sto_cmd(REL_TEXTDEF, hash);
					} else if (p[NTYPE] == UDEF) {
						if ((curseg != UDEFSEG) || (p[NVALUE] != curpos[UDEFSEG]))
							if (pass == 1)
								error("multiply defined");
							else
								error("phase error");
						if (ext && (pass == 2))
							sto_cmd(REL_UDEFDEF, hash);
					} else {
						error("not implemented");
					}
					continue;
				} else if (match("=")) {
					if (pass == 1)
						error("multiply defined");
					else
						expression(lval);
					break;
				} else {
					error("internal error");
					ch = 0;
					break;
				}
			case OPCODE:
				do_opcode(p);
				break;
			case PSEUDO:
				if (p[NVALUE] == PSEUDO_END)
					return; // don't read next line keeping file open
				do_pseudo(p);
				break;
			case LINK:
				if (pass == 1)
					error("multiply defined");
				ch = 0; // errors displayed in pass 1
				break;
			default:
				if (pass == 1)
					error("opcode expected");
				break;
			}

			// test for junk
			blanks();
			if (ch)
				error("encountered junk");
			break;
		}
		preprocess();  // fetch next line
	}
}
