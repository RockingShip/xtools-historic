#include <stdio.h>
#include "system.hpp"
#include "cpu.hpp"
#include "kernel.hpp"
#include "opcodes.hpp"

// KERNEL is a global object
extern KERNEL kernel;

BOOLEAN CPU::read_byte (ADDRESS addr,
                        char *value)
{
  // read memory location
  if (!pagetable->read_byte (addr, value)) {
    // call kernel with error status
    context.setpc (startpc);
    kernel.event (1, addr);
    return FALSE;
  }
  return TRUE;
}

BOOLEAN CPU::read_word (ADDRESS addr,
                        int *value)
{
  // read memory location
  if (!pagetable->read_word (addr, value)) {
    // call kernel with error status
    context.setpc (startpc);
    kernel.event (1, addr);
    return FALSE;
  }
  return TRUE;
}

BOOLEAN CPU::write_byte (ADDRESS addr,
                         char value)
{
  // read memory location
  if (!pagetable->write_byte (addr, value)) {
    // call kernel with error status
    context.setpc (startpc);
    kernel.event (1, addr);
    return FALSE;
  }
  return TRUE;
}

BOOLEAN CPU::write_word (ADDRESS addr,
                         int value)
{
  // write memory location
  if (!pagetable->write_word (addr, value)) {
    // call kernel with error status
    context.setpc (startpc);
    kernel.event (1, addr);
    return FALSE;
  }
  return TRUE;
}

BOOLEAN CPU::get_pc_byte (char *value)
{
ADDRESS addr;

  // get address PC is pointing to
  addr = context.getpc ();
  // read memory location
  if (!read_byte (addr, value))
    return FALSE;
  // update PC
  context.setpc (context.getpc () + 1);  // increment PC with 1
  return TRUE;
}

BOOLEAN CPU::get_pc_word (int *value)
{
ADDRESS addr;

  // get address PC is pointing to
  addr = context.getpc ();
  // read memory location
  if (!read_word (addr, value))
    return FALSE;
  // update PC
  context.setpc (context.getpc () + 2);  // increment PC with 2
  return TRUE;
}

BOOLEAN CPU::get_ea (int *value)
{
int imm, lval, rval;
char lreg, rreg;

  // get immediate value
  if (!get_pc_word (&imm))
    return FALSE;
  // get left register
  if (!get_pc_byte (&lreg))
    return FALSE;
  // get right register
  if (!get_pc_byte (&rreg))
    return FALSE;
  // if lreg non-zero, then read contence
  lval = (lreg) ? context.getreg (lreg) : 0;
  // if rreg non-zero, then read contence
  rval = (rreg) ? context.getreg (rreg) : 0;
  // merge everything into a result
  *value = imm + lval + rval;
  return TRUE;
}

CPU::tick ()
{
int addr, spaddr, val, result;
int i, R0, R1;
char opcode, lreg, rreg, ch;

 if(0){
  int val, base, i, j;

  // generate a code dump
  for (j=0; j<6; j++)
    if (pagetable->read_byte (context.getpc ()+j, &ch))
      printf (" %02x", ch);
    else
      printf (" **");
  printf ("\n");

  // generate a register dump
  printf ("PC:%04x", context.getpc());
  for (i=0; i<8; i++)
    printf (" R%2d:%04x", i, context.getreg(i));
  printf ("\n");
  printf ("CC:%04x" , context.getcc());
  for (i=8; i<16; i++)
    printf (" R%2d:%04x", i, context.getreg(i));
  printf ("\n");
  }

  // get next opcode
  startpc = context.getpc ();
  if (!get_pc_byte (&opcode))
    return;

  // decode opcode
  switch (opcode) {
    case LDW: // LDW R,M
      // load operands
      if (get_pc_byte (&lreg) && get_ea (&addr))
        // read word pointed to by EA addr
        if (read_word (addr, &val))
          // save found value into register (set CC in alu)
          context.setreg (lreg, alu.calc (LDR, 0, val));
      break;
    case STW: // STW R,M
      // load operands
      if (get_pc_byte (&lreg) && get_ea (&addr))
        // write word pointed to by EA addr (set CC in alu)
        write_word (addr, alu.calc (LDR, 0, context.getreg (lreg)));
      break;
    case LDA: // LDA R,M
      // load operands
      if (get_pc_byte (&lreg) && get_ea (&addr))
        // save found value into register (set CC in alu)
        context.setreg (lreg, alu.calc (LDR, 0, addr));
      break;
    case LDB: // LDB R,M
      // load operands
      if (get_pc_byte (&lreg) && get_ea (&addr))
        // read word pointed to by EA addr
        if (read_byte (addr, &ch))
          // save found value into register (set CC in alu)
          context.setreg (lreg, alu.calc (LDR, 0, ch));
      break;
    case STB: // STB R,M
      // load operands
      if (get_pc_byte (&lreg) && get_ea (&addr))
        // write word pointed to by EA addr (set CC in alu)
        write_byte (addr, alu.calc (LDR, 0, context.getreg (lreg)));
      break;
    case SVC: // SVC I
      // load SVC opcode
      if (get_pc_word (&val)) {
        // get contence of R0 and R1
        R0 = context.getreg (0);
        R1 = context.getreg (1);
        if (kernel.supervisor (val, &R0, &R1)) {
          // restore values of R0 and R1
          context.setreg (0, R0);
          context.setreg (1, R1);
        } else {
          // SVC not implemented
          context.setpc (startpc);
          kernel.event (2, startpc);
        }
      }
      break;
    case CMP: // CMP R,R
      // load operands
      if (get_pc_byte (&lreg) && get_pc_byte (&rreg))
        // execute a dummy SUB to set CC bits
        result = alu.calc (SUB, context.getreg (lreg), context.getreg (rreg));
      break;
    case LDR: // LDR R,R
    case SUB: // SUB R,R
    case ADD: // ADD R,R
    case MUL: // MUL R,R
    case DIV: // DIV R,R
    case MOD: // MOD R,R
    case OR : // OR  R,R
    case AND: // AND R,R
    case XOR: // XOR R,R
    case ASL: // ASL R,R
    case ASR: // ASR R,R
      // load operands
      if (get_pc_byte (&lreg) && get_pc_byte (&rreg)) {
        // execute operation, and set CC bits
        result = alu.calc (opcode, context.getreg (lreg), context.getreg (rreg));
        // store result in register
        context.setreg (lreg, result); // store result
      }
      break;
    case NOT: // NOT R
    case NEG: // NEG R
      // load operands
      if (get_pc_byte (&lreg)) {
        // execute operation, and set CC bits
        result = alu.calc (opcode, context.getreg (lreg), 0);
        // store result in register
        context.setreg (lreg, result); // store result
      }
      break;
    case JSB: // JSB M
      // load operands
      if (get_ea (&addr)) {
      // execute EA calculation of -(sp)
      spaddr = context.getreg(15) - 2;
      context.setreg (15, spaddr);
      // write PC to (SP)
      if (write_word (spaddr, context.getpc ()))
        // setup new PC
        context.setpc (addr);
      }
      break;
    case RSB: // RSB
      // execute EA calculation of (sp)+
      spaddr = context.getreg(15);
      context.setreg (15, spaddr+2);
      // read (SP) into val
      if (read_word (spaddr, &val))
        // jump to saved address
        context.setpc (val);
      break;
    case PSHR: // PSHR I
      // load register mask
      if (get_pc_word (&addr)) {
        for (i=0; i<15; i++)
          // test if bit in mask
          if (addr & (1<<i)) {
            // push register onto stack

            // execute EA calculation of -(sp)
            spaddr = context.getreg(15) - 2;
            context.setreg (15, spaddr);
            // write reg to (SP)
            if (!write_word (spaddr, context.getreg (i)))
              break;  // error occured, stop loop
          }
      }
      break;
    case POPR: // POPR I
      // load register mask
      if (get_pc_word (&addr)) {
        for (i=14; i>=0; i--)
          // test if bit in mask
          if (addr & (1<<i)) {
            // pop register from stack

            // execute EA calculation of (sp)+
            spaddr = context.getreg(15);
            context.setreg (15, spaddr+2);
            // load reg from (SP)
            if (!read_word (spaddr, &val))
              break;  // error occured, stop loop
            else
              context.setreg (i, val);
          }
      }
      break;
    case PSHB: // PSHB M
      // load operands
      if (get_ea (&addr)) {
      // get contence of addr
        if (read_byte (addr, &ch)) {
          // execute EA calculation of -(sp)
          spaddr = context.getreg(15) - 2;
          context.setreg (15, spaddr);
          // write word pointed to by EA to (SP)
          write_word (spaddr, alu.calc (LDR, 0, ch));
        }
      }
      break;
    case PSHW: // PSHW M
      // load operands
      if (get_ea (&addr)) {
      // get contence of addr
        if (read_word (addr, &val)) {
          // execute EA calculation of -(sp)
          spaddr = context.getreg(15) - 2;
          context.setreg (15, spaddr);
          // write word pointed to by EA to (SP)
          write_word (spaddr, alu.calc (LDR, 0, val));
        }
      }
      break;
    case PSHA: // PSHA M
      // load operands
      if (get_ea (&addr)) {
        // execute EA calculation of -(sp)
        spaddr = context.getreg(15) - 2;
        context.setreg (15, spaddr);
        // write EA to (SP)
        write_word (spaddr, alu.calc (LDR, 0, addr));
      }
      break;
    case BLE: // BLE M
    case BLT: // BLT M
    case BGE: // BGE M
    case BGT: // BGT M
    case BNE: // BNE M
    case BEQ: // BEQ M
      if (get_ea (&addr))           // get jump address
        if (alu.test_cc (opcode))
          context.setpc (addr);     // jump if condition valid
      break;
    case JMP: // JMP M
      if (get_ea (&addr))           // get jump address
        context.setpc (addr);       // always jump
      break;
    default:  // unimplemented opcode
      context.setpc (startpc);
      kernel.event (2, startpc);
      break;
  }
}

BOOLEAN CPU::loadfile (char *fname)
{
  pagetable->loadfile (fname);
}

CPU::load_context (CPU_CONTEXT *context,
                   PAGE_TABLE  *pagetable)
{
  // load context
  CPU::context = *context;
  CPU::pagetable = pagetable;
  // store CC in ALU
  alu.setcc (CPU::context.getcc ());
}

CPU::save_context (CPU_CONTEXT *context)
{
  // retrieve CC from alu
  CPU::context.setcc (alu.getcc ());
  // return context
  *context = CPU::context;
}
