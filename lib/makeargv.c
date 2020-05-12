/*
** Convert a string into a ARGC ARGV definition
*/
make_argv (str, argc, argv)
register char *str;
register int *argc, *argv;
{
  /* no items found yet */
  *argc = 0;

  while (*str) {
    /* skip spaces */
    while (*str && (*str <= ' '))
      ++str;

    /* item found ? */
    if (*str) {
      /* mark item */
      ++*argc;
      *argv++ = str;
      /* skip until space */
      while (*str && (*str > ' '))
        ++str;
      /* terminate string */
      if (*str)
        *str++ = 0;
    }
  }
}