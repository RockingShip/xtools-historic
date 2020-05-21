int stdin = 0;
int stdout = 1;
int stderr = 2;

osprint(char *str) {
	asm("lda r1,4(r14)");
	asm("svc 31");
}

fread(char *buf, int siz, int cnt, int hdl) {
	int ctrl[4];

	ctrl[0] = buf;
	ctrl[1] = siz;
	ctrl[2] = cnt;
	ctrl[3] = hdl;

	asm("lda r1,0(r15)");
	return asm("svc 40");
}

fwrite(int buf, int siz, int cnt, int hdl) {
	int ctrl[4];

	ctrl[0] = buf;
	ctrl[1] = siz;
	ctrl[2] = cnt;
	ctrl[3] = hdl;

	asm("lda r1,0(r15)");
	return asm("svc 41");
}

fopen(char *name, char *mode) {
	int ctrl[2];

	ctrl[0] = name;
	ctrl[1] = mode;

	asm("lda r1,0(r15)");
	return asm("svc 42");
}

fclose(int hdl) {
	int ctrl[1];

	ctrl[0] = hdl;

	asm("lda r1,0(r15)");
	return asm("svc 43");
}

fseek(int hdl, int pos, int whence) {
	int ctrl[3];

	ctrl[0] = hdl;
	ctrl[1] = pos;
	ctrl[2] = whence;

	asm("lda r1,0(r15)");
	return asm("svc 44");
}

ftell(int hdl) {
	int ctrl[1];

	ctrl[0] = hdl;

	asm("lda r1,0(r15)");
	return asm("svc 47");
}

unlink(char *name) {
	int ctrl[1];

	ctrl[0] = name;

	asm("lda r1,0(r15)");
	return asm("svc 45");
}

rename(char *old, char *new) {
	int ctrl[2];

	ctrl[0] = old;
	ctrl[1] = new;

	asm("lda r1,0(r15)");
	return asm("svc 46");
}

exit(int code) {
	int ctrl[1];

	ctrl[0] = code;

	asm("lda r1,0(r15)");
	asm("svc 99");
}
