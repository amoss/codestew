#include <stdio.h>
#include <stdlib.h>
#include "arm.h"
#include "addProj.h"

#define W 32
static Type  *Word   = new Type(Type::UBITS,W);
static Type  *Flag   = new Type(Type::UBITS,1);


Projection *ArmMachine::translate(Block *block)
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

char const* regNames[] = {
  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"
};
int numRegs = 10;
std::string ArmMachine::outCodeworks(Block *block)
{
  printf("OUTPUT\n");
  // Trivial register allocator
  if(block->numValues() > numRegs)
    return std::string("// TRIVIAL FAILED");
  printf("HERE?\n");
std::vector<Instruction*> schedule = block->topSort();
  for(int i=0; i<schedule.size(); i++)
  {
    printf("SLOT %d\n",i);
    printf("%s %s,%s,%s\n", schedule[i]->opcode->name, 
           regNames[schedule[i]->inputs[0]->ref],
           regNames[schedule[i]->inputs[1]->ref],
           regNames[schedule[i]->outputs[0]->ref]);
  }
  return std::string();
}

static bool trivial(Allocation *regAlloc)
{
  // Check if value set is small enough for trivial allocation
  int regsNeeded = 0;
  for(int i=0; i<regAlloc->regs.size(); i++)
    if( regAlloc->regs[i]==NULL )
      regsNeeded++;
  if(regsNeeded <= numRegs)
  {
    int regCounter=0;
    for(int i=0; i<regAlloc->regs.size(); i++)
      if(regAlloc->regs[i]==NULL)
        regAlloc->regs[i] = regNames[regCounter++];
    return true;
  }
  printf("Trivial failed %d/%d\n",regsNeeded,numRegs);
  return false;
}

static bool modulo(Allocation *regAlloc)
{
  return false;
}

Allocation *ArmMachine::allocate(Block *block)
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

  if( trivial(regAlloc) )
    return regAlloc; 
  if( modulo(regAlloc) )
    return regAlloc;
  printf("Modulo failed\n");
  exit(-1);
  return NULL;
}
