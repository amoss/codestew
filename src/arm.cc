#include <map>
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

static char const* armRegNames[] = {
  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9",
  "r10", "r11" // , "r12"
};

ArmMachine::ArmMachine()
{
  this->numRegs = 12;
  this->regNames = armRegNames;
}

std::string ArmMachine::outCodeworks(Allocation *alloc)
{
  for(int i=0; i<alloc->numValues(); i++)
    printf("//Val %d: %s\n", i, alloc->regs[i]);
  for(int i=0; i<alloc->schedule.size(); i++)
  {
    if( !strcmp(alloc->schedule[i]->opcode->name, "addco") )
    {
      printf("  add %s,%s,%s\n",
             regNames[alloc->schedule[i]->inputs[0]->ref],
             regNames[alloc->schedule[i]->inputs[1]->ref],
             regNames[alloc->schedule[i]->outputs[0]->ref]);
    }
    else if( !strcmp(alloc->schedule[i]->opcode->name, "addcico") )
    {
      printf("  adc %s,%s,%s\n",
             regNames[alloc->schedule[i]->inputs[0]->ref],
             regNames[alloc->schedule[i]->inputs[1]->ref],
             regNames[alloc->schedule[i]->outputs[0]->ref]);
    }
    else if( !strcmp(alloc->schedule[i]->opcode->name, "signext") )
    {
      printf("  eor %s, %s, %s\n",
             alloc->regs[ alloc->schedule[i]->outputs[0]->ref ],
             alloc->regs[ alloc->schedule[i]->outputs[0]->ref ],
             alloc->regs[ alloc->schedule[i]->outputs[0]->ref ]);
      printf("  adc %s,%s,#0\n",
             alloc->regs[alloc->schedule[i]->inputs[0]->ref],
             alloc->regs[alloc->schedule[i]->outputs[0]->ref]);
    }
    else
    {
      printf("SLOT %d\n",i);
      printf("%s %s,%s,%s\n", alloc->schedule[i]->opcode->name, 
             regNames[alloc->schedule[i]->inputs[0]->ref],
             regNames[alloc->schedule[i]->inputs[1]->ref],
             regNames[alloc->schedule[i]->outputs[0]->ref]);
    }
  }
  return std::string();
}
