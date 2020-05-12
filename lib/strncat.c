/*
** concatenate n bytes max from t to end of s 
** s must be large enough
*/
strncat (s, t, n)
register char *s, *t;
register int n;
{
register char *d;

  d = s;
  --s;
  while (*++s) ;
  while (n--) {
    if (*s++ = *t++)
      continue;
    return d;
  }
  *s = 0;
  return d;
}
