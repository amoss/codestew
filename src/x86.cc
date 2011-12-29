#include <stdio.h>
#include <stdlib.h>
#include "x86.h"
#include "addProj.h"




#define W 64

/*struct Interval
{
  Value *source;
  uint32 lo, size;
  Value *target;
}; */

static Type  *Word   = new Type(Type::UBITS,W);
static Type  *Flag   = new Type(Type::UBITS,1);

Projection *X86Machine::translate(Block *block)
{
Projection *p = newValSplit(block,W,*this);

  std::vector<Instruction*> order = block->topSort();
  printf("Trans: %zu insts\n",order.size());
  for(int i=0; i<order.size(); i++)
  {
    Instruction *inst = order[i];
    if(!strcmp(inst->opcode->name, "add"))
      translateUbitAdd(inst, p, (Machine*)this);
    else if(!strcmp(inst->opcode->name, "xor")) {
      printf("XOR projection unimplemented\n");
      return NULL;
    }
    else {
      printf("ERROR: Cannot translate\n");
      exit(-1);
    }
  }
  return p;
}

static char const *x86RegNames[] =
{
  "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14"
};
#define NUMREGS (sizeof(regNames)/sizeof(char const *))

X86Machine::X86Machine()
{
  this->numRegs = NUMREGS;
  this->regNames = x86RegNames;
}

static bool opCoalesces(Opcode *op)
{
  return !strcmp("addco",op->name)   || !strcmp("addzo",op->name) ||
         !strcmp("addcico",op->name) || !strcmp("addcizo",op->name) ;
}

void X86Machine::preAllocActions(Allocation *alloc)
{
  // Check for coalesced regs first as reduces reg-pressure
  for(int i=0; i<alloc->numValues(); i++)
  {
    Value *v = alloc->getValue(i);
    if(v->defined() && opCoalesces(v->def->opcode) && v==v->def->outputs[0])
      alloc->regs[i] = "implied";
  }
}

std::string X86Machine::outGccInline(Allocation *alloc)
{
std::string result;
char line[120];
  result += "__asm__ __volatile__(\"\\\n";
  for(int i=0; i<alloc->regs.size(); i++)
  {
    sprintf(line, "// REGMAP %d : %s\\\n", i, alloc->regs[i]);
    result += line;
  }
  std::vector<Instruction*> order = alloc->topSort();
  printf("SIZE %zu\n",order.size());
  for(int i=0; i<order.size(); i++)
  {
    if( !strcmp(order[i]->opcode->name, "addco") ) {
      sprintf(line,"  addq %%%s, %%%s;\n", alloc->regs[ order[i]->inputs[0]->ref ], 
                                       alloc->regs[ order[i]->inputs[1]->ref ]);
      result += line;
    }
    else if( !strcmp(order[i]->opcode->name, "addcico") ) {
      sprintf(line,"  adcq %%%s, %%%s;\n", alloc->regs[ order[i]->inputs[0]->ref ], 
                                       alloc->regs[ order[i]->inputs[1]->ref ]);
      result += line;
    }
    else if( !strcmp(order[i]->opcode->name, "signext") &&
             !strcmp("carry",alloc->regs[order[i]->inputs[0]->ref]) ) {
      const char *regName = alloc->regs[ order[i]->outputs[0]->ref ];
      sprintf(line,"  xorq %%%s, %%%s;\n  adc #0, %%%s\n",
                   regName, regName, regName);
      result += line;
    }
    else {
      sprintf(line,"OTHER %s\n",order[i]->opcode->name);
      result += line;
    }
    
  }
  result += "\");\n";
  return result;
}

bool X86Machine::valid(Block *block)
{
  return false;
}
