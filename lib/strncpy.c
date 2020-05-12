/*
** copy n characters from sour to dest (null padding)
*/
strncpy (dest, sour, n)
register char *dest, *sour;
register int n;
{
register char *d;

  d = dest;
  while (n-- > 0) {
    if (*d++ = *sour++)
      continue;
    while (n-- > 0)
      *d++ = 0;
  }
  *d = 0;
  return dest;
}

