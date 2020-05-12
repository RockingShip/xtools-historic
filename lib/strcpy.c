/*
** copy t to s 
*/
strcpy (s, t)
register char *s, *t;
{
register char *d;

  d = s;
  while (*s++ = *t++) ;
  return d;
}

