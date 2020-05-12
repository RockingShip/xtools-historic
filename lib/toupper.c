/*
** return upper-case of c if it is lower-case, else c
*/
toupper (c)
register int c;
{
  return (c >= 'a' && c <= 'z') ? c+('A'-'a') : c;
}

