/*
** return lower-case of c if upper-case, else c
*/
tolower (c)
register int c;
{
  return (c >= 'A' && c <= 'Z') ? c+('a'-'A') : c;
}

