#ifndef ALU_HPP
#define ALU_HPP

#include "system.hpp"

class ALU
{
  private :
    int cc;

  public :
    int calc (int operation,
              int lvalue,
              int rvalue);
    BOOLEAN test_cc (int operation);
    int getcc ();
    setcc (int value);
};

#endif
