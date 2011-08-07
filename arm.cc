#include "arm.h"

Block *ArmMachine::translate(SimpleMachine *source, Block *block)
{
  // Most basic translate is just a deep-copy of the block, no attempt at filtering
  // or converting yet
Block *result = block->clone();
  return result;
}

char const* regNames[] = {
  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"
};
int numRegs = 10;
std::string ArmMachine::outCodeworks(Block *block)
{
  // Trivial register allocator
  if(block->numValues() > numRegs)
    return false;
std::vector<Instruction*> schedule = block->topSort();
  for(int i=0; i<schedule.size(); i++)
  {
    printf("%s %s,%s,%s\n", schedule[i]->opcode->name, 
           regNames[schedule[i]->inputs[0]->ref],
           regNames[schedule[i]->inputs[1]->ref],
           regNames[schedule[i]->outputs[0]->ref]);
  }
  return std::string();
}

bool trivial(Block *block, int numRegs, char const**regNames)
{

  return true;
}
