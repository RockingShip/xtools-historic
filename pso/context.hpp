#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "system.hpp"

class CPU_CONTEXT
{

  private :
    int regs[16];
    int pc;
    int cc;

  public :
    CPU_CONTEXT ();

    int getreg (int reg);
    int getpc ();
    int getcc ();

    setreg (int reg,
            int value);
    setpc (int value);
    setcc (int value);
};

#endif
