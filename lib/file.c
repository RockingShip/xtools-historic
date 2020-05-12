fread (hdl, buf, len)
int hdl, len;
char *buf;
{
int ctrl[4];
  
  ctrl[0] = hdl;
  ctrl[1] = buf;
  ctrl[2] = len;
#asm
	lda	r1,0(r15)
	svc	40
	stw	r1,0(r15)
#endasm
  return ctrl[3];
}

fwrite (hdl, buf, len)
int hdl, len;
char *buf;
{
int ctrl[4];
  
  ctrl[0] = hdl;
  ctrl[1] = buf;
  ctrl[2] = len;
#asm
	lda	r1,0(r15)
	svc	41
	stw	r1,0(r15)
#endasm
  return ctrl[3];
}

fopen (fname, mode, type)
char *fname;
int mode, type;
{
int ctrl[4];
  
  ctrl[1] = fname;
  ctrl[2] = mode;
  ctrl[3] = type;
#asm
	lda	r1,0(r15)
	svc	42
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

fclose (hdl)
int hdl;
{
int ctrl[4];
  
  ctrl[0] = hdl;
#asm
	lda	r1,0(r15)
	svc	43
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

fseek (hdl, pos)
int hdl, pos;
{
int ctrl[4];
  
  ctrl[0] = hdl;
  ctrl[1] = pos;
#asm
	lda	r1,0(r15)
	svc	44
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

fdelete (fn)
char *fn;
{
int ctrl[4];
  
  ctrl[0] = fn;
#asm
	lda	r1,0(r15)
	svc	45
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}

frename (ofn, nfn)
char *ofn, *nfn;
{
int ctrl[4];
  
  ctrl[0] = ofn;
  ctrl[1] = nfn;
#asm
	lda	r1,0(r15)
	svc	46
	stw	r1,0(r15)
#endasm
  return ctrl[0];
}
