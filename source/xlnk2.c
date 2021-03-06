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
//* X-Linker.  Part 2, Link control
//*

#define EXTERN extern
#include "xlnk.h"

objerr(register int *fp, char *msg, int curseg) {
	printf(msg);
	printf(" at ");
	if (curseg == CODESEG)
		printf("C%04x", fp[FCODEPOS]);
	else if (curseg == DATASEG)
		printf("D%04x", fp[FDATAPOS]);
	else if (curseg == TEXTSEG)
		printf("D%04x", fp[FTEXTPOS]);
	else
		printf("U%04x", fp[FUDEFPOS]);
	printf(" in %s\n", inpfn);
	errflag = 1;
}

/*
 * Unsigned compare GT
 * Trying all possibilities reveals:
 * unsigned "i>j" can be rewritten as "(j^i)&0x8000 ?  i&0x8000 : (j-i)&0x8000"
 */
unsigned_GT(int i, int j) {
	return ((j ^ i) & (1 << SBIT) ? i & (1 << SBIT) : (j - i) & (1 << SBIT));
}

save_seg_size(register int *fp) {
	if (unsigned_GT(fp[FCODEPOS], fp[FCODELEN]))
		fp[FCODELEN] = fp[FCODEPOS];
	if (unsigned_GT(fp[FDATAPOS], fp[FDATALEN]))
		fp[FDATALEN] = fp[FDATAPOS];
	if (unsigned_GT(fp[FTEXTPOS], fp[FTEXTLEN]))
		fp[FTEXTLEN] = fp[FTEXTPOS];
	if (unsigned_GT(fp[FUDEFPOS], fp[FUDEFLEN]))
		fp[FUDEFLEN] = fp[FUDEFPOS];
}

read_byte() {
	char arr[1];

	if (fread(arr, 1, 1, inphdl) != 1)
		fatal("missing .END (use -v to discover where)\n");

	// return unsigned
	return arr[0] & 0xff;
}

read_word() {
	char arr[2];
	int w;

	if (fread(arr, 1, 2, inphdl) != 2)
		fatal("missing .END (use -v to discover where)\n");

	// return signed
	w = arr[0] << 8 | (arr[1] & 0xff);
	w |= -(w & (1 << SBIT));
	return w;
}

write_byte(char byte) {
	fwrite(&byte, 1, 1, outhdl);
}

write_word(int word) {
	char arr[2];

	arr[0] = word >> 8;
	arr[1] = word;

	fwrite(arr, 1, 2, outhdl);
}


dopass1(int fileid, int libid, int libofs) {
	register int *p, *fp, curseg;
	int symseg, symofs, hash;
	char cmd;

	fp = &file2[file2inx++ * FLAST];
	if (file2inx >= FILEMAX)
		fatal("too many files");
	fp[FFILE] = fileid;
	fp[FLIB] = libid;
	fp[FOFFSET] = libofs;
	fp[FCODEBASE] = fp[FCODEPOS] = fp[FCODELEN] = 0;
	fp[FDATABASE] = fp[FDATAPOS] = fp[FDATALEN] = 0;
	fp[FTEXTBASE] = fp[FTEXTPOS] = fp[FTEXTLEN] = 0;
	fp[FUDEFBASE] = fp[FUDEFPOS] = fp[FUDEFLEN] = 0;

	curseg = CODESEG;
	if (verbose) {
		soutname(fileid, datbuf);
		printf("processing %s", datbuf);
		if (libid != -1) {
			soutname(libid, datbuf);
			printf(" (%s)", datbuf);
		}
		printf("\n", datbuf);
	}
	while (1) {
		cmd = read_byte();
		if (cmd < 0) {
			datlen = -cmd;
			fread(datbuf, 1, datlen, inphdl);
			if (curseg == CODESEG)
				fp[FCODEPOS] += datlen;
			else if (curseg == DATASEG)
				fp[FDATAPOS] += datlen;
			else if (curseg == TEXTSEG)
				fp[FTEXTPOS] += datlen;
			else
				fp[FUDEFPOS] += datlen;
		} else {
			switch (cmd) {
			case REL_ADD:
			case REL_SUB:
			case REL_MUL:
			case REL_DIV:
			case REL_MOD:
			case REL_LSR:
			case REL_LSL:
			case REL_XOR:
			case REL_AND:
			case REL_OR:
			case REL_NOT:
			case REL_NEG:
			case REL_SWAP:
				// skip basic stack operations
				break;
			case REL_PUSHB:
			case REL_CODEB:
			case REL_DATAB:
			case REL_TEXTB:
			case REL_UDEFB:
				// skip byte length stack operations
				read_byte();
				break;
			case REL_PUSHW:
			case REL_CODEW:
			case REL_DATAW:
			case REL_TEXTW:
			case REL_UDEFW:
				// skip BPW length stack operations
				read_word();
				break;
			case REL_SYMBOL:
				// Push symbol value on stack
				datlen = read_byte(); // length
				fread(datbuf, 1, datlen, inphdl); // symbol
				datbuf[datlen] = 0;
				dohash(datbuf, &hash);
				p = &name[hash * NLAST];
				if (!p[NTYPE])
					p[NTYPE] = UNDEF;
				if (debug)
					printf("SYMBOL: %s\n", datbuf);
				break;
			case REL_POPB:
				// increase curpos with 1
				if (curseg == CODESEG)
					fp[FCODEPOS] += 1;
				else if (curseg == DATASEG)
					fp[FDATAPOS] += 1;
				else if (curseg == TEXTSEG)
					fp[FTEXTPOS] += 1;
				else
					fp[FUDEFPOS] += 1;
				break;
			case REL_POPW:
				// increase curpos with BPW
				if (curseg == CODESEG)
					fp[FCODEPOS] += BPW;
				else if (curseg == DATASEG)
					fp[FDATAPOS] += BPW;
				else if (curseg == TEXTSEG)
					fp[FTEXTPOS] += BPW;
				else
					fp[FUDEFPOS] += BPW;
				break;
			case REL_DSB:
				// skip specified number of bytes in current segment
				symofs = read_word(); // skipcount
				if (curseg == CODESEG)
					fp[FCODEPOS] += symofs;
				else if (curseg == DATASEG)
					fp[FDATAPOS] += symofs;
				else if (curseg == TEXTSEG)
					fp[FTEXTPOS] += symofs;
				else
					fp[FUDEFPOS] += symofs;
				break;
			case REL_END:
				save_seg_size(fp);
				return;
			case REL_CODEDEF:
			case REL_DATADEF:
			case REL_TEXTDEF:
			case REL_UDEFDEF:
				// symbol definition
				if (cmd == REL_CODEDEF)
					symseg = CODE;
				else if (cmd == REL_DATADEF)
					symseg = DATA;
				else if (cmd == REL_TEXTDEF)
					symseg = TEXT;
				else
					symseg = UDEF;
				symofs = read_word(); // symbol offset
				datlen = read_byte(); // length
				fread(datbuf, 1, datlen, inphdl); // symbol
				datbuf[datlen] = 0;
				dohash(datbuf, &hash);
				p = &name[hash * NLAST];
				if (!p[NTYPE])
					p[NTYPE] = UNDEF;
				if (p[NTYPE] == UNDEF) {
					// new symbol
					p[NTYPE] = symseg;
					p[NMODULE] = file2inx - 1;
					p[NVALUE] = symofs;
				} else {
					printf("Symbol '%s' doubly defined", datbuf);
					objerr(fp, "", symseg);
				}
				if (debug)
					printf("SYMDEF: %s %d:%d\n", datbuf, symseg, symofs);
				break;
			case REL_CODEORG:
			case REL_DATAORG:
			case REL_TEXTORG:
			case REL_UDEFORG:
				symofs = read_word(); // segment offset
				save_seg_size(fp);
				if (cmd == REL_CODEORG) {
					curseg = CODESEG;
					fp[FCODEPOS] = symofs;
				} else if (cmd == REL_DATAORG) {
					curseg = DATASEG;
					fp[FDATAPOS] = symofs;
				} else if (cmd == REL_TEXTORG) {
					curseg = TEXTSEG;
					fp[FTEXTPOS] = symofs;
				} else {
					curseg = UDEFSEG;
					fp[FUDEFPOS] = symofs;
				}
				if (debug)
					printf("ORG: %d:%d\n", curseg, symofs);
				break;
			default:
				printf("unknown command %d", cmd);
				objerr(fp, "", curseg);
				exit(1);
				break;
			}
		}
	}
}

dopass2(register int *fp) {
	register int i, lval, rval;
	int curseg, symofs, hash, val;
	char cmd, cval;

	curseg = CODESEG;
	stackinx = 0;
	fseek(outhdl, fp[FCODEBASE], 0);

	if (verbose) {
		soutname(fp[FFILE], datbuf);
		printf("processing %s", datbuf);
		if (fp[FLIB] != -1) {
			soutname(fp[FLIB], datbuf);
			printf(" (%s)", datbuf);
		}
		printf("\n", datbuf);
	}

	while (1) {
		cmd = read_byte();
		if (cmd < 0) {
			datlen = -cmd;
			fread(datbuf, 1, datlen, inphdl);
			fwrite(datbuf, 1, datlen, outhdl);
			if (curseg == CODESEG)
				fp[FCODEPOS] += datlen;
			else if (curseg == DATASEG)
				fp[FDATAPOS] += datlen;
			else if (curseg == TEXTSEG)
				fp[FTEXTPOS] += datlen;
			else {
				objerr(fp, "data in UDEF segment", curseg);
				exit(1);
			}
		} else {
			switch (cmd) {
				case REL_ADD:
				case REL_SUB:
				case REL_MUL:
				case REL_DIV:
				case REL_MOD:
				case REL_LSR:
				case REL_LSL:
				case REL_XOR:
				case REL_AND:
				case REL_OR:
					rval = stack[--stackinx];
					lval = stack[--stackinx];
					if (stackinx < 0) objerr(fp, "stack underflow", curseg);
					switch (cmd) {
						case REL_ADD:
							lval += rval;
							break;
						case REL_SUB:
							lval -= rval;
							break;
						case REL_MUL:
							lval *= rval;
							break;
						case REL_DIV:
							lval /= rval;
							break;
						case REL_MOD:
							lval %= rval;
							break;
						case REL_LSR:
							lval >>= rval;
							break;
						case REL_LSL:
							lval <<= rval;
							break;
						case REL_XOR:
							lval ^= rval;
							break;
						case REL_AND:
							lval &= rval;
							break;
						case REL_OR:
							lval |= rval;
							break;
					}
					stack[stackinx++] = lval;
					break;
				case REL_NOT:
				case REL_NEG:
					lval = stack[--stackinx];
					if (stackinx < 0) objerr(fp, "stack underflow", curseg);
					if (cmd == REL_NOT)
						lval = ~lval;
					else
						lval = -lval;
					stack[stackinx++] = lval;
					break;
				case REL_SWAP:
					lval = stack[--stackinx];
					rval = stack[--stackinx];
					if (stackinx < 0) objerr(fp, "stack underflow", curseg);
					stack[stackinx++] = lval;
					stack[stackinx++] = rval;
					break;
				case REL_PUSHB:
				case REL_CODEB:
				case REL_DATAB:
				case REL_TEXTB:
				case REL_UDEFB:
					val = read_byte();
					if (cmd == REL_PUSHB)
						stack[stackinx] = val;
					else if (cmd == REL_CODEB)
						stack[stackinx] = fp[FCODEBASE] + val;
					else if (cmd == REL_DATAB)
						stack[stackinx] = fp[FDATABASE] + val;
					else if (cmd == REL_TEXTB)
						stack[stackinx] = fp[FTEXTBASE] + val;
					else
						stack[stackinx] = fp[FUDEFBASE] + val;
					stackinx += 1;
					if (stackinx >= STACKMAX) objerr(fp, "stack overflow", curseg);
					break;
				case REL_PUSHW:
				case REL_CODEW:
				case REL_DATAW:
				case REL_TEXTW:
				case REL_UDEFW:
					val = read_word();
					if (cmd == REL_PUSHW)
						stack[stackinx] = val;
					else if (cmd == REL_CODEW)
						stack[stackinx] = fp[FCODEBASE] + val;
					else if (cmd == REL_DATAW)
						stack[stackinx] = fp[FDATABASE] + val;
					else if (cmd == REL_TEXTW)
						stack[stackinx] = fp[FTEXTBASE] + val;
					else
						stack[stackinx] = fp[FUDEFBASE] + val;
					stackinx += 1;
					if (stackinx >= STACKMAX) objerr(fp, "stack overflow", curseg);
					break;
				case REL_SYMBOL:
					// Push symbol value on stack
					datlen = read_byte(); // length
					fread(datbuf, 1, datlen, inphdl); // symbol
					datbuf[datlen] = 0;
					dohash(datbuf, &hash);

					stack[stackinx++] = name[hash * NLAST + NVALUE];
					if (stackinx >= STACKMAX) objerr(fp, "stack overflow", curseg);
					break;
				case REL_POPB:
					// pop byte from stack
					stackinx -= 1;
					if (stackinx < 0) objerr(fp, "stack underflow", curseg);
					if ((stack[stackinx] < -128) || (stack[stackinx] > 127))
						objerr(fp, "byte overflow", curseg);
					write_byte(stack[stackinx]);

					// increase curpos with 1
					if (curseg == CODESEG)
						fp[FCODEPOS] += 1;
					else if (curseg == DATASEG)
						fp[FDATAPOS] += 1;
					else if (curseg == TEXTSEG)
						fp[FTEXTPOS] += 1;
					else {
						objerr(fp, "data in UDEF segment", curseg);
						exit(1);
					}
					break;
				case REL_POPW:
					// pop word from stack
					stackinx -= 1;
					if (stackinx < 0) objerr(fp, "stack underflow", curseg);
					write_word(stack[stackinx]);

					// increase curpos with BPW
					if (curseg == CODESEG)
						fp[FCODEPOS] += BPW;
					else if (curseg == DATASEG)
						fp[FDATAPOS] += BPW;
					else if (curseg == TEXTSEG)
						fp[FTEXTPOS] += BPW;
					else {
						objerr(fp, "data in UDEF segment", curseg);
						exit(1);
					}
					break;
				case REL_DSB:
					// skip specified number of bytes in current segment
					symofs = read_word(); // skipcount
					if (curseg == CODESEG) {
						fp[FCODEPOS] += symofs;
						i = symofs;
					} else if (curseg == DATASEG) {
						fp[FDATAPOS] += symofs;
						i = symofs;
					} else if (curseg == TEXTSEG) {
						fp[FTEXTPOS] += symofs;
						i = symofs;
					} else {
						fp[FUDEFPOS] += symofs;
						i = 0;
					}

					datbuf[0] = datbuf[1] = datbuf[2] = datbuf[3] = 0;
					while (i > 4) {
						fwrite(datbuf, 1, 4, outhdl);
						i -= 4;
					}
					while (i > 0) {
						fwrite(datbuf, 1, 1, outhdl);
						i -= 1;
					}
					break;
				case REL_END:
					if (stackinx) {
						printf("stack not properly released in %s\n", inpfn);
						exit(1);
					}
					return;
				case REL_CODEDEF:
				case REL_DATADEF:
				case REL_TEXTDEF:
				case REL_UDEFDEF:
					// symbol definition (skipped in pass2)
					symofs = read_word() & 0xffff; // symbol offset
					datlen = read_byte() & 0xff; // length
					fread(datbuf, 1, datlen, inphdl); // symbol
					break;
				case REL_CODEORG:
				case REL_DATAORG:
				case REL_TEXTORG:
				case REL_UDEFORG:
					symofs = read_word(); // segment offset
					if (cmd == REL_CODEORG) {
						curseg = CODESEG;
						fp[FCODEPOS] = symofs;
						i = fp[FCODEBASE] + symofs;
					} else if (cmd == REL_DATAORG) {
						curseg = DATASEG;
						fp[FDATAPOS] = symofs;
						i = fp[FDATABASE] + symofs;
					} else if (cmd == REL_TEXTORG) {
						curseg = TEXTSEG;
						fp[FTEXTPOS] = symofs;
						i = fp[FTEXTBASE] + symofs;
					} else {
						curseg = UDEFSEG;
						fp[FUDEFPOS] = symofs;
						i = 0;
					}
					fseek(outhdl, i, 0);
					break;
				default:
					printf("unknown command %d", cmd);
					objerr(fp, "", curseg);
					exit(1);
					break;
			}
		}
	}
}

doreloc() {
	register int i, curpos, curlen, *p;
	int hash;

	dohash("___CODEBASE", &hash);
	name[hash * NLAST + NVALUE] = 0;
	curpos = 4;  // Reserve space for JMP ___START
	curlen = 0;

	// relocate CODE
	for (i = 0; i < file2inx; ++i) {
		p = &file2[i * FLAST];
		if (p[FFILE] != -1) {
			p[FCODEBASE] = curpos;
			curpos += p[FCODELEN];
			curlen += p[FCODELEN];
		}
	}

	dohash("___CODELEN", &hash);
	name[hash * NLAST + NVALUE] = curlen;
	dohash("___DATABASE", &hash);
	name[hash * NLAST + NVALUE] = curpos;
	curlen = 0;

	// relocate DATA
	for (i = 0; i < file2inx; ++i) {
		p = &file2[i * FLAST];
		if (p[FFILE] != -1) {
			p[FDATABASE] = curpos;
			curpos += p[FDATALEN];
			curlen += p[FDATALEN];
		}
	}

	dohash("___DATALEN", &hash);
	name[hash * NLAST + NVALUE] = curlen;
	dohash("___TEXTBASE", &hash);
	name[hash * NLAST + NVALUE] = curpos;
	curlen = 0;

	// relocate TEXT
	for (i = 0; i < file2inx; ++i) {
		p = &file2[i * FLAST];
		if (p[FFILE] != -1) {
			p[FTEXTBASE] = curpos;
			curpos += p[FTEXTLEN];
			curlen += p[FTEXTLEN];
		}
	}

	dohash("___TEXTLEN", &hash);
	name[hash * NLAST + NVALUE] = curlen;
	dohash("___UDEFBASE", &hash);
	name[hash * NLAST + NVALUE] = curpos;
	curlen = 0;

	// relocate UDEF
	for (i = 0; i < file2inx; ++i) {
		p = &file2[i * FLAST];
		if (p[FFILE] != -1) {
			p[FUDEFBASE] = curpos;
			curpos += p[FUDEFLEN];
			curlen += p[FUDEFLEN];
		}
	}

	dohash("___UDEFLEN", &hash);
	name[hash * NLAST + NVALUE] = curlen;

	// redefine ___STACKLEN
	dohash("___STACKLEN", &hash);
	p = &name[hash * NLAST];
	p[NVALUE] = stksiz;

	// relocate all symbols
	for (i = 0; i < NAMEMAX; ++i) {
		p = &name[i * NLAST];
		if (p[NTYPE] == CODE)
			p[NVALUE] += file2[p[NMODULE] * FLAST + FCODEBASE];
		else if (p[NTYPE] == DATA)
			p[NVALUE] += file2[p[NMODULE] * FLAST + FDATABASE];
		else if (p[NTYPE] == TEXT)
			p[NVALUE] += file2[p[NMODULE] * FLAST + FTEXTBASE];
		else if (p[NTYPE] == UDEF)
			p[NVALUE] += file2[p[NMODULE] * FLAST + FUDEFBASE];
	}
}

process() {
	register int i, j, *fp, *p, len;
	int hash, found;

	pass = 1;
	if (verbose)
		printf("Pass 1\n");

	// process pass 1
	for (i = 0; i < file1inx; ++i) {
		fp = &file1[i * FLAST];
		if (fp[FLIB] == -1) {
			// process object
			soutname(fp[FFILE], inpfn);
			inphdl = open_file(inpfn, "r");
			dopass1(fp[FFILE], fp[FLIB], 0);
			fclose(inphdl);
		} else {
			// process library
			soutname(fp[FLIB], inpfn);
			open_olb();
			found = 0;
			while (1) {
				for (j = 0; j < NAMEMAX; ++j) {
					p = &name[j * NLAST];
					if (p[NTYPE] == UNDEF) {
						// found undefined symbol, test if in library
						soutname(j, datbuf);
						lbdohash(datbuf, &hash);
						p = &lbname[hash * LBNLAST];
						if (p[LBNLIB] != -1) {
							// found module containing symbol definition
							p = &lbfile[p[LBNLIB] * LBFLAST];
							fseek(inphdl, p[LBFOFFSET], 0);
							// add module name to linker symboltable
							lbsoutname(p[LBFNAME], datbuf);
							dohash(datbuf, &hash);
							// process
							dopass1(hash, fp[FLIB], p[LBFOFFSET]);
							found = 1;
						}
					}
				}
				// if all symbols tested, test for another load
				if (!found)
					break;
				found = 0;
			}
			fclose(inphdl);
		}
	}

	// relocate modules
	doreloc();

	// test for undefined symbols
	if (!undef) {
		j = 0;
		for (i = 0; i < NAMEMAX; ++i) {
			p = &name[i * NLAST];
			if (p[NTYPE] == UNDEF) {
				if (!j) {
					printf("Undefined symbols :\n");
					errflag = 1;
					j = 1;
				}
				outname(i);
				printf("\n");
			}
		}
	}

	pass = 2;
	if (verbose)
		printf("Pass 2\n");

	// generate prefix "JMP ___START"
	datbuf[0] = 0x6F;  // opcode for JMP
	dohash("___START", &hash);
	datbuf[1] = name[hash * NLAST + NVALUE] >> 8; // hi
	datbuf[2] = name[hash * NLAST + NVALUE]; // lo
	datbuf[3] = 0; // reg
	fwrite(datbuf, 1, 4, outhdl);

	// process pass 2
	for (i = 0; i < file2inx; ++i) {
		fp = &file2[i * FLAST];
		if (fp[FLIB] == -1) {
			// process object
			soutname(fp[FFILE], inpfn);
			inphdl = open_file(inpfn, "r");
			dopass2(fp);
			fclose(inphdl);
		} else {
			// process library
			soutname(fp[FLIB], inpfn);
			inphdl = open_file(inpfn, "r");
			while (1) {
				fseek(inphdl, fp[FOFFSET], 0);
				dopass2(fp);
				// test if next module comes from same library
				if (((i + 1) >= file2inx) || (file2[(i + 1) * FLAST + FLIB] != fp[FLIB]))
					break;
				// reinit fp and redo loop
				fp = &file2[++i * FLAST];
			}
			fclose(inphdl);
		}
	}
}
