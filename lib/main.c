_get_cmdline (str)
char *str;
{
int ctrl[3];
  
  ctrl[0] = str;
#asm
	lda	r0,0x32
	lda	r1,0(r15)
	svc	90
#endasm
}

_main()
{
char cmdline[128];

  _get_cmdline (cmdline);
  main (cmdline);
  exit (0);
}

exit (code)
int code;
{
  SVC (99);
}