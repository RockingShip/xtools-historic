int stdin = 0;
int stdout = 1;
int stderr = 2;

osprint (str)
char *str;
{
#asm
	lda	r1,4(r14)
	svc	31
#endasm
}

/* vararg oscall */
SVC(id)
int id;
{
int ctrl[1];

#asm
	lda	r1,4(r14)
	stw	r1,lbl+1
lbl:	svc	0
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

fread (buf, siz, cnt, hdl)
char *buf;
int siz, hdl, cnt;
{
int ctrl[4];
  
  ctrl[0] = buf;
  ctrl[1] = siz;
  ctrl[2] = cnt;
  ctrl[3] = hdl;
#asm
	lda	r1,0(r15)
	svc	40
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

fwrite (buf, siz, cnt, hdl)
int buf, siz, cnt, hdl;
{
int ctrl[4];
  
  ctrl[0] = buf;
  ctrl[1] = siz;
  ctrl[2] = cnt;
  ctrl[3] = hdl;
#asm
	lda	r1,0(r15)
	svc	41
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

fopen(name, mode)
char *name, *mode;
{
int ctrl[2];
  
  ctrl[0] = name;
  ctrl[1] = mode;
#asm
	lda	r1,0(r15)
	svc	42
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

fclose(hdl)
char hdl;
{
int ctrl[1];
  
  ctrl[0] = hdl;
#asm
	lda	r1,0(r15)
	svc	43
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

fseek(hdl, pos, whence)
int hdl, pos, whence;
{
int ctrl[3];
  
  ctrl[0] = hdl;
  ctrl[1] = pos;
  ctrl[2] = whence;
#asm
	lda	r1,0(r15)
	svc	44
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

ftell(hdl)
int hdl;
{
int ctrl[1];

ctrl[0] = hdl;
#asm
lda	r1,0(r15)
svc	47
stw	r1,0(r15)
#endasm
return ctrl[0];
}

unlink(name)
char *name;
{
int ctrl[1];
  
  ctrl[0] = name;
#asm
	lda	r1,0(r15)
	svc	45
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

rename(old, new)
char *old, *new;
{
int ctrl[2];
  
  ctrl[0] = old;
  ctrl[1] = new;
#asm
	lda	r1,0(r15)
	svc	46
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

exit(code)
int code;
{
int ctrl[1];
  
  ctrl[0] = code;
#asm
	lda	r1,0(r15)
	svc	99
#endasm
}
