#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "system.hpp"
#include "pcb.hpp"
#include "mmu.hpp"

class KERNEL
{
  private:
    BOOLEAN read_byte (ADDRESS addr,
                       char *value);
    BOOLEAN read_word (ADDRESS addr,
                       int *value);
    BOOLEAN write_byte (ADDRESS addr,
                        char value);
    BOOLEAN write_word (ADDRESS addr,
                        int value);
    BOOLEAN copystr (ADDRESS addr, char *buf);

  public:
    PCB *current;

    event (int event,
           int value);
    supervisor (int opcode,
                int *R0,
                int *R1);
};

#endif
