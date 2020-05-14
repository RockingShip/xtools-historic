#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "file.hpp"

XFILE::XFILE(PAGE_TABLE *pagetable) {
	XFILE::pagetable = pagetable;
	mode = 0;
	type = 0;
}

int XFILE::fopen(char *fname, char *mode) {
	char *cp;

	/* Open file */
	if (mode[0] == 'w' && mode[1] == 0) {
		hdl = open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
		if (hdl < 0)
			return hdl;
		this->mode = 'W';
	} else if (mode[0] == 'r' && mode[1] == 0) {
		hdl = open(fname, O_RDWR, 0);
		if (hdl < 0)
			return hdl;
		this->mode = 'R';
	} else {
		printf("file '%s' has unknown open mode '%c'\n", fname, mode);
		exit(1);
	}

	strncpy(name, fname, PATHMAX);

	bufpos = filpos = buflen = bufofs = 0;
	dirty  = 0;

	/* initial load */
	if (this->mode == 'R')
		fload();

	return 0;
}

int XFILE::fflush() {
	/* reposition if needed */
	if (bufpos != filpos) {
		if (lseek(hdl, bufpos, SEEK_SET) != bufpos) {
			printf("seek error on file '%s'\n", name);
			exit(1);
		}
	}
	/* writeback */
	if (write(hdl, buf, buflen) != buflen) {
		printf("write error on file '%s'\n", name);
		exit(1);
	}
	filpos = bufpos + buflen;
	/* clear bit */
	dirty = 0;
}

int XFILE::fload() {
	/* load window */
	if (bufpos != filpos) {
		if (lseek(hdl, bufpos, SEEK_SET) != bufpos) {
			printf("seek error on file '%s'\n", name);
			exit(1);
		}
	}
	if ((buflen = read(hdl, buf, BUFMAX)) < 0) {
		printf("read error on file '%s'\n", name);
		perror(">>");
		exit(1);
	}
	filpos = bufpos + buflen;
}

int XFILE::fread(ADDRESS addr, int siz, int cnt) {

	if (!mode || !siz)
		return -1;

	if (mode == 'W') {
		printf("file '%s' not opened for read\n", name);
		exit(1);
	}

	int len = siz * cnt;
	int retlen = 0;
	while (len) {
		/* test if extractpoint in window */
		if (bufofs >= buflen) {
			/* update dirty buffer */
			if (dirty)
				fflush();
			/* init window */
			bufpos += buflen;
			bufofs = 0;
			fload();
			/* test for EOF */
			if (!buflen)
				return retlen ? retlen / siz : 0 /*EOF*/ ;
		}
		/* extract from window */
		pagetable->write_byte(addr++, buf[bufofs++]);
		--len;
		++retlen;
	}
	return retlen / siz;
}

int XFILE::fwrite(ADDRESS addr, int siz, int cnt) {
	int retlen;
	char ch;

	if (!mode || !siz)
		return -1;

	if (mode == 'R') {
		printf("file '%s' not opened for write\n", name);
		exit(1);
	}

	int len = siz * cnt;
	retlen = 0;
	while (len) {
		/* test if insertpoint in window */
		if (bufofs >= BUFMAX) {
			/* update dirty buffer */
			buflen = BUFMAX;
			if (dirty)
				fflush();
			/* init window */
			bufpos += BUFMAX;
			bufofs = buflen = 0;
			if (mode == 'U')
				fload();
		}
		dirty = 1;

		pagetable->read_byte(addr++, &buf[bufofs++]);
		--len;
		++retlen;

		if (bufofs > buflen)
			buflen = bufofs;
	}
	return retlen / siz;
}

int XFILE::fseek(int pos, int whence) {
	int i;
	long len;
	char *cp;

	if (!mode)
		return -1;
	if (whence != SEEK_SET && whence != SEEK_CUR)
		return -1;

	/* convert SEEK_CUR to SEEK_SET */
	if (whence == SEEK_CUR) {
		whence = SEEK_SET;
		pos = bufpos + bufofs + pos;
	}

	/* test if positioned within current block */
	if ((pos - bufpos >= 0) && (pos - bufpos < BUFMAX)) {
		bufofs = pos - bufpos;
		if (bufofs - buflen >= 0)
			buflen = bufofs + 1;
		return pos;
	}

	/* first flush buffer if dirty */
	if (dirty)
		fflush();

	/* now do a OS seek */
	while (filpos != pos) {
		filpos = lseek(hdl, pos, SEEK_SET);
		if (filpos != pos) {
			/* erase buffer */
			for (i = 0, cp = buf; i < BUFMAX; i++)
				*cp++ = 0;
			/* go to EOF */
			filpos = lseek(hdl, 0,2);
			/* fill */
			while (filpos - pos < 0) {
				len = pos - filpos;
				if (len >= BUFMAX)
					len = BUFMAX;
				if (write(hdl, buf, len) != len) {
					printf("write error on file '%s'\n", name);
					exit(1);
				}
				filpos += len;
			}
			filpos = 0; /* redo loop */
		}
	}

	/* read buffer */
	bufpos = pos;
	bufofs = buflen = 0;
	fload();

	return pos;
}

int XFILE::ftell() {
	return bufpos + bufofs;
}

int XFILE::fclose() {
	int i;

	if (!mode)
		return -1;

	if (dirty)
		fflush();

	close(hdl);
	hdl = 0;

	return 0;
}
