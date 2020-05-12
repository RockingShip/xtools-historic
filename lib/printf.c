#define BPW 2

_puts (str)
char *str;
{
#asm
	lda	r1,4(r14)
	svc	31
#endasm
}

_doprt (optr, cmd, args)
register char *optr, *cmd;
int *args;
{
register char  ch;
register char  *nptr;
register int   val;
int            i;
int            justify, fld_len, minint, negative;
char           leading;
char           numbuf[32];

  while (ch = *cmd++) {
    if (ch=='\n') {
      *optr++ = '\15';
      *optr++ = '\12';
    } else if (ch != '%') {
      *optr++ = ch;
    } else {
      ch = *cmd++;

      justify = 0;
      if (ch == '-') {
        justify++;
        ch = *cmd++;
      }
      leading = ' ';
      if (ch == '0') {
        leading = ch;
        ch = *cmd++;
      }
      fld_len = 0;
      if (ch == '*') {
         fld_len = *--args;
         ch = *cmd++;
      } else {
         while ((ch>='0') && (ch<='9')) {
           fld_len = fld_len * 10 + ch - '0';
           ch = *cmd++;
         }
      }

      nptr = numbuf;
      switch (ch) {
        case 'x':
          val = *--args;
          i = fld_len;
          do {
            ch = val&0xF;
            if (ch <= 9)
              ch += '0';
            else
              ch += 'A'-10;
            *nptr++ = ch;
            val >>= 4;
          } while ((--i > 0) || ((val) && ((val != -1) || (ch < '8'))));
          break;
        case 'd':
          val = *--args;
          minint = 0;
          if (val == 0x8000) {
            minint = 1;
            val++;
          }
          negative = 0;
          if (val < 0) {
            negative = 1;
            val = -val;
          }
          do {
            ch = val % 10;
            *nptr++ = ch + '0';
            val = val / 10;
          } while (val);
          if (negative)
            *nptr++ = '-';
          if (minint)
            numbuf[0]++;
          break;
        case 's':
          nptr = *--args;
          while (ch = *nptr++)
            if (ch == '\n') {
              *optr++ = '\15';
              *optr++ = '\12';
            } else
              *optr++ = ch;
          nptr = numbuf; /* Reset pointer */
          break;
        case 'c':
          *optr++ = *--args;
          break;
        default:
          *optr++ = ch;
          break;
      }

      /* Some numeric postprocessing */
      fld_len -= nptr-numbuf;  /* Get length of filler */

      if (justify)
        while (fld_len-- > 0)
          *optr++ = leading;
      while (nptr != numbuf)
        *optr++ = *--nptr;
      while (fld_len-- > 0)
        *optr++ = leading;
    }

  }
  *optr++ = 0;
}

printf (anchor)
int anchor;
{
char obuf[256];
register char *cmd;
register int *args;

  args = &anchor+ARGC*BPW;
  cmd = *--args;
  _doprt (obuf, cmd, args);
  _puts (obuf);
}

fprintf (anchor)
int anchor;
{
char obuf[256];
register char *cmd;
register int hdl, *args;

  args = &anchor+ARGC*BPW;
  hdl = *--args;
  cmd = *--args;
  _doprt (obuf, cmd, args);
  fwrite (hdl, obuf, 0);
}

sprintf (anchor)
int anchor;
{
register char *cmd, *buf;
register int hdl, *args;

  args = &anchor+ARGC*BPW;
  buf = *--args;
  cmd = *--args;
  _doprt (buf, cmd, args);
}
