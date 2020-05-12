#include "system.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "mmu.hpp"
#include "kernel.hpp"
#include "cpu.hpp"

extern CPU cpu;

BOOLEAN KERNEL::read_byte (ADDRESS addr,
                           char *value)
{
  // read memory location
  if (!current->pagetable.read_byte (addr, value)) {
    // call kernel with error status
    event (1, addr);
    return FALSE;
  }
  return TRUE;
}

BOOLEAN KERNEL::read_word (ADDRESS addr,
                           int *value)
{
  // read memory location
  if (!current->pagetable.read_word (addr, value)) {
    // call kernel with error status
    event (1, addr);
    return FALSE;
  }
  return TRUE;
}

BOOLEAN KERNEL::write_byte (ADDRESS addr,
                            char value)
{
  // read memory location
  if (!current->pagetable.write_byte (addr, value)) {
    // call kernel with error status
    event (1, addr);
    return FALSE;
  }
  return TRUE;
}

BOOLEAN KERNEL::write_word (ADDRESS addr,
                            int value)
{
  // read memory location
  if (!current->pagetable.write_word (addr, value)) {
    // call kernel with error status
    event (1, addr);
    return FALSE;
  }
  return TRUE;
}

KERNEL::event (int event,
               int value)
{
CPU_CONTEXT context;
int val, base, i, j;

  printf ("\nKernel event %d (parm:%04x)\n", event, value);

  // generate a register dump
  cpu.save_context (&context);
  printf ("PC:%04x", context.getpc());
  for (i=0; i<8; i++)
    printf (" R%2d:%04x", i, context.getreg(i));
  printf ("\n");
  printf ("CC:%04x" , context.getcc());
  for (i=8; i<16; i++)
    printf (" R%2d:%04x", i, context.getreg(i));
  printf ("\n");

  printf ("\nCode dump : \n");
  base = context.getpc ();
  for (i=0; i<4; i++) {
    printf ("%04x:", base+i*16);
    for (j=0; j<16; j+=2)
      if (current->pagetable.read_word (base+i*16+j, &val))
        printf (" %04x", val);
      else
        printf (" ****");
    printf ("\n");
  }

  printf ("\nStack dump : \n");
  base = context.getreg (15);
  for (i=0; i<4; i++) {
    printf ("%04x:", base+i*16);
    for (j=0; j<16; j+=2)
      if (current->pagetable.read_word (base+i*16+j, &val))
        printf (" %04x", val);
      else
        printf (" ****");
    printf ("\n");
  }

  exit (0);
}

BOOLEAN KERNEL::copystr (ADDRESS addr, char *buf)
{
char ch;

  while (read_byte (addr++, &ch)) {
    *buf++ = ch;
    if (!ch)
      return;
  }
}

BOOLEAN KERNEL::supervisor (int opcode,
                            int *R0,
                            int *R1)
{
char buf[512], buf2[80];
int i, ch, addr, hdl, len, pos, retlen, mode, type;
char *strp;

  switch (opcode) {
    case 31: // TPUT
      // get start address of string
      if (read_word (*R1+0, &addr)) {
        copystr (addr, buf);
        printf (buf);
      }
      return TRUE;
    case 40: // DGET
      read_word (*R1+0, &hdl);
      read_word (*R1+2, &addr);
      read_word (*R1+4, &len);
      retlen = current->fread (hdl, addr, len);
      write_word (*R1+6, retlen);
      *R1 = retlen;
      return TRUE;
    case 41: // DPUT
      read_word (*R1+0, &hdl);
      read_word (*R1+2, &addr);
      read_word (*R1+4, &len);
      retlen = current->fwrite (hdl, addr, len);
      write_word (*R1+6, retlen);
      *R1 = retlen;
      return TRUE;
    case 42: // DOPEN
      read_word (*R1+2, &addr);
      copystr (addr, buf);
      read_word (*R1+4, &mode);
      read_word (*R1+6, &type);
      hdl = current->fopen (buf, mode, type);
      write_word (*R1+0, hdl);
      *R1 = hdl;
      return TRUE;
    case 43: // DCLOSE
      read_word (*R1+0, &hdl);
      retlen = current->fclose (hdl);
      *R1 = retlen;
      return TRUE;
    case 44: /* FSEEK */
      read_word (*R1+0, &hdl);
      read_word (*R1+2, &pos);
      retlen = current->fseek (hdl, pos);
      write_word (*R1+0, retlen);
      *R1 = retlen;
      return TRUE;
    case 45: /* FDELETE */
      read_word (*R1+0, &addr);
      copystr (addr, buf);
      retlen = unlink (buf);
      write_word (*R1+0, retlen);
      *R1 = retlen;
      return TRUE;
    case 46: /* FRENAME */
      read_word (*R1+0, &addr);
      copystr (addr, buf);
      read_word (*R1+2, &addr);
      copystr (addr, buf2);
      retlen = rename (buf, buf2);
      write_word (*R1+0, retlen);
      *R1 = retlen;
      return TRUE;
    case 90: // OSINFO
      switch (*R0) {
        case 0x0032:  // Get commandline
          // return zero length
          read_word (*R1+0, &addr);
          write_word (*R1+4, 6);
          for (i=0; i<7; i++)
            write_byte (addr++, current->command[i]);
          return TRUE;
        default:
          return FALSE;
      }
    case 99: // RETURN
      current->exit ();
      exit (0);
    default:
      return FALSE;
  }
}
