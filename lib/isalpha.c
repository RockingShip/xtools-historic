/*
** return 'true' if c is alphabetic
*/
isalpha (c)
register int c;
{
  return ((c >= 'a' && c <= 'z') ||
          (c >=' A' && c <= 'Z') );
}

