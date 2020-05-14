#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cpu.hpp"
#include "kernel.hpp"
#include "mmu.hpp"
#include "system.hpp"

extern CPU cpu;

BOOLEAN KERNEL::read_byte(ADDRESS addr, char *value) {
	// read memory location
	if (!current->pagetable.read_byte(addr, value)) {
		// call kernel with error status
		event(1, addr);
		return FALSE;
	}
	return TRUE;
}

BOOLEAN KERNEL::read_word(ADDRESS addr, int *value) {
	// read memory location
	if (!current->pagetable.read_word(addr, value)) {
		// call kernel with error status
		event(1, addr);
		return FALSE;
	}
	return TRUE;
}

BOOLEAN KERNEL::write_byte(ADDRESS addr, char value) {
	// read memory location
	if (!current->pagetable.write_byte(addr, value)) {
		// call kernel with error status
		event(1, addr);
		return FALSE;
	}
	return TRUE;
}

BOOLEAN KERNEL::write_word(ADDRESS addr, int value) {
	// read memory location
	if (!current->pagetable.write_word(addr, value)) {
		// call kernel with error status
		event(1, addr);
		return FALSE;
	}
	return TRUE;
}

void KERNEL::event(int event, int value) {
	CPU_CONTEXT context;
	int val, base, i, j;
	char cval;

	printf("\nKernel event %d (parm:%04x)\n", event, value);

	// generate a register dump
	cpu.save_context(&context);
	printf("PC:%04x", context.getpc() & 0xffff);
	for (i = 0; i < 8; i++)
		printf(" R%2d:%04x", i, context.getreg(i) & 0xffff);
	printf("\n");
	printf("CC:%04x", context.getcc());
	for (i = 8; i < 16; i++)
		printf(" R%2d:%04x", i, context.getreg(i) & 0xffff);
	printf("\n");

	printf("\nCode dump : \n");
	base = context.getpc();
	for (i = 0; i < 4; i++) {
		printf("%02x:", base + i * 16);
		for (j = 0; j < 16; j++)
			if (current->pagetable.read_byte(base + i * 16 + j, &cval))
				printf(" %02x", cval & 0xff);
			else
				printf(" **");
		printf("\n");
	}

	printf("\nStack dump : \n");
	base = context.getreg(15) & 0xffff;
	for (i = 0; i < 4; i++) {
		printf("%04x:", base + i * 16);
		for (j = 0; j < 16; j += 2)
			if (current->pagetable.read_word(base + i * 16 + j, &val))
				printf(" %04x", val & 0xffff);
			else
				printf(" ****");
		printf("\n");
	}

	exit(0);
}

void KERNEL::copystr(ADDRESS addr, char *buf) {
	char ch;

	while (read_byte(addr++, &ch)) {
		*buf++ = ch;
		if (!ch)
			return;
	}
}

BOOLEAN KERNEL::supervisor(int opcode, int *R0, int *R1) {
	char buf[512], buf2[80];

	switch (opcode) {
		case 31: { // osprint()
			int str;

			// get start address of string
			if (read_word(*R1 + 0, &str)) {
				copystr(str, buf);
				fputs(buf, stdout);
			}

			return TRUE;
		}
		case 40: { /* fread() */
		        int buf, siz, cnt, hdl;

			read_word(*R1 + 0, &buf);
			read_word(*R1 + 2, &siz);
			read_word(*R1 + 4, &cnt);
			read_word(*R1 + 6, &hdl);

			*R1 = current->fread(buf, siz, cnt, hdl);
			return TRUE;
		}
		case 41: { /* fwrite() */
		        int buf, siz, cnt, hdl;

			read_word(*R1 + 0, &buf);
			read_word(*R1 + 2, &siz);
			read_word(*R1 + 4, &cnt);
			read_word(*R1 + 6, &hdl);

			*R1 = current->fwrite(buf, siz, cnt, hdl);
			return TRUE;
		}
		case 42: { /* fopen() */
			int name, mode;

			read_word(*R1 + 0, &name);
			read_word(*R1 + 2, &mode);
			copystr(name, buf);
			copystr(mode, buf2);

			*R1 = current->fopen(buf, buf2);
			return TRUE;
		}
		case 43: { /* fclose() */
			int hdl;

			read_word(*R1 + 0, &hdl);

			*R1 = current->fclose(hdl);
			return TRUE;
		}
		case 44: { /* fseek() */
			int hdl, ofs, whence;

			read_word(*R1 + 0, &hdl);
			read_word(*R1 + 2, &ofs);
			read_word(*R1 + 4, &whence);

			/* sign extend */
			ofs |= -(ofs & (1 << 15));

			*R1 = current->fseek(hdl, ofs, whence);
			return TRUE;
		}
		case 45: { /* unlink() */
			int name;

			read_word(*R1 + 0, &name);
			copystr(name, buf);

			*R1 =  unlink(buf);
			return TRUE;
		}
		case 46: { /* rename() */
			int oldname, newname;

			read_word(*R1 + 0, &oldname);
			read_word(*R1 + 2, &newname);
			copystr(oldname, buf);
			copystr(newname, buf2);

			*R1 = rename(buf, buf2);
			return TRUE;
		}
		case 47: { /* ftell() */
			int hdl;

			read_word(*R1 + 0, &hdl);

			*R1 = current->ftell(hdl);
			return TRUE;
		}
		case 90: // OSINFO
			switch (*R0) {
				case 0x0032: { // Get commandline
					int addr, i;

					// return zero length
					read_word(*R1 + 0, &addr);
					write_word(*R1 + 4, 6);
					for (i = 0; i < 7; i++)
						write_byte(addr++, current->command[i]);
					return TRUE;
				}
				default:
					return FALSE;
			}
		case 99: { /* exit() */
			int code;

			read_word(*R1 + 0, &code);

			current->exit();
			exit(code);
		}
		default:
			return FALSE;
	}
}
