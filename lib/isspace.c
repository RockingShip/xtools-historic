/*
** return 'true' if c is a white-space character
*/
isspace (c)
register int c;
{
  /* first check gives quick exit in most cases */
  return (c <= ' ' && (c == ' ' || (c <= 13 && c >= 9)));
}

