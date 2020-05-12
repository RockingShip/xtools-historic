/*
** return length of s 
*/
strlen (s)
register char *s;
{
register char *t;

  t = s - 1;
  while (*++t) ;
  return t-s;
}

