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
Projection *p = newValSplit(block,W);

  std::vector<Instruction*> order = block->topSort();
  printf("Trans: %zu insts\n",order.size());
  for(int i=0; i<order.size(); i++)
  {
    Instruction *inst = order[i];
    if(!strcmp(inst->opcode->name, "add"))
      translateUbitAdd(inst, p, (Machine*)this);
    else {
      printf("ERROR: Cannot translate\n");
      exit(-1);
    }
  }
  return p;
}

static char const *regNames[] =
{
  "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14"
};
#define NUMREGS (sizeof(regNames)/sizeof(char const *))

static bool opCoalesces(Opcode *op)
{
  return !strcmp("addco",op->name)   || !strcmp("addzo",op->name) ||
         !strcmp("addcico",op->name) || !strcmp("addcizo",op->name) ;
}

Allocation *X86Machine::allocate(Block *block)
{
Allocation *regAlloc = new Allocation(block);
  // Presize / initial state is NULL 
  for(int i=0; i<block->numValues(); i++)
    regAlloc->regs.push_back(NULL);

  // Nail up fixed registers based on instruction types
  for(int i=0; i<block->numInsts(); i++)
  {
    Instruction *inst = block->getInst(i);
    if( !strcmp(inst->opcode->name,"addco") || !strcmp(inst->opcode->name,"addcico")) {
      regAlloc->regs[ inst->outputs[1]->ref ] = "carry";
    }
  }

  // Check for coalesced regs first as reduces reg-pressure
  for(int i=0; i<block->numValues(); i++)
  {
    Value *v = block->getValue(i);
    if(v->defined() && opCoalesces(v->def->opcode) && v==v->def->outputs[0])
      regAlloc->regs[i] = "implied";
  }

  // Check if value set is small enough for trivial allocation
  int regsNeeded = 0;
  for(int i=0; i<regAlloc->regs.size(); i++)
    if( regAlloc->regs[i]==NULL )
      regsNeeded++;
  if(regsNeeded <= NUMREGS)
  {
    int regCounter=0;
    for(int i=0; i<regAlloc->regs.size(); i++)
      if(regAlloc->regs[i]==NULL)
        regAlloc->regs[i] = regNames[regCounter++];
    for(int i=0; i<regAlloc->regs.size(); i++)
    {
      if(!strcmp(regAlloc->regs[i], "implied"))
      {
        printf("Coalescing %u\n",i);
        Value *v = block->getValue(i);
        Instruction *inst = v->def;
        printf("From %llu\n",inst->ref);
        Value *overwritten = inst->inputs[1];
        regAlloc->regs[i] = regAlloc->regs[overwritten->ref];
      }
    }
    return regAlloc;
  }
  printf("Trivial failed\n");
  exit(-1);
  return NULL;
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
