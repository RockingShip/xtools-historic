/*
** return 'true' if c is a graphic character
** (33-126)
*/
isgraph (c)
register int c;
{
  return (c >= 33 && c <= 126);
}

