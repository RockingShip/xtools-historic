/*
** reverse string in place 
*/
reverse (s)
register char *s;
{
register char *j;
register int c;

  j = s + strlen(s) - 1;
  while (s < j) {
    c = *s;
    *s++ = *j;
    *j-- = c;
  }
}
