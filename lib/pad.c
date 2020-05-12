/*
** Place n occurrences of ch at dest.
*/
pad (dest, ch, n)
register  char *dest, *n;
register  int ch;
{
  while (n--)
    *dest++ = ch;
}

