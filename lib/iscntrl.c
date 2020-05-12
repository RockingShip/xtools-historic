/*
** return 'true' if c is a control character
** (0-31 or 127)
*/
iscntrl (c)
register char *c;
{
  return ((c >= 0 && c <= 31) || (c == 127));
}

