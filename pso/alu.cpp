#include "system.hpp"
#include "alu.hpp"
#include "opcodes.hpp"

int ALU::calc (int operation,
               int lval,
               int rval)
{
int retval;

  switch (operation) {
    case LDR: retval =          rval; break;
    case SUB: retval =  lval -  rval; break;
    case ADD: retval =  lval +  rval; break;
    case MUL: retval =  lval *  rval; break;
    case DIV: retval =  lval /  rval; break;
    case MOD: retval =  lval %  rval; break;
    case OR:  retval =  lval |  rval; break;
    case AND: retval =  lval &  rval; break;
    case XOR: retval =  lval ^  rval; break;
    case ASL: retval =  lval << rval; break;
    case ASR: retval =  lval >> rval; break;
    case NOT: retval = ~lval;         break;
    case NEG: retval = -lval;         break;
    default:  retval =  0;
  }

  // setup cc register according to retval
  if (retval == 0)
    cc = 0;
  else if (retval < 0)
    cc = 1;
  else
    cc = 2;

  // return found value to caller
  return retval;
}

int ALU::test_cc (int operation)
{
  switch (operation) {
    case BLE:  return (cc != 2);
    case BLT:  return (cc == 1);
    case BGE:  return (cc != 1);
    case BGT:  return (cc == 2);
    case BNE:  return (cc != 0);
    case BEQ:  return (cc == 0);
    default:   return FALSE;
  }
}

ALU::setcc (int value)
{
  cc = value;
}

int ALU::getcc ()
{
  return cc;
}
