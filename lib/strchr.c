/*
** return pointer to 1st occurrence of c in str, else 0
*/
strchr(str, c)
register char *str, c;
{
  while (*str) {
    if (*str == c)
       return str;
    ++str;
  }
  return 0;
}

