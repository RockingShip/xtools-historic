#include <bios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.hpp"
#include "mmu.hpp"
#include "kernel.hpp"
#include "cpu.hpp"

KERNEL kernel;
CPU cpu;

main (int argc, char *argv[])
{
int i;

  kernel.current = new PCB ();
  // generate complete commandline
  for (i=2; i<argc; i++) {
    if (i != 2)
      strcat (kernel.current->command, " ");
    strcat (kernel.current->command, argv[i]);
  }
  cpu.load_context (&kernel.current->context, &kernel.current->pagetable);

/*
  for (i=0; i<100000; i++)
    if (bioskey (1) && ((_bios_keybrd(0)&0xFF) == 4)) exit (0);
*/

  printf ("loading...\n");
  cpu.loadfile (strcat (argv[1], ".IMG"));
  printf ("starting...\n");
  while (1)
    cpu.tick ();
}
