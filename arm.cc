#include <stdio.h>
#include "arm.h"

#define W 32
static Type  *Word   = new Type(Type::UBITS,W);
static Type  *Flag   = new Type(Type::UBITS,1);
static Opcode opcodes[] =
{
#define ADDCO 0
  Opcode("addco",2,2),  // Add with carry out :           Word,Word -> Word, Bit
#define ADDZO 1
  Opcode("addzo",2,1),  // Add with zero out :            Word,Word -> Word
#define ADDCICO 2
  Opcode("addcico",3,2),// Add with carry in, carry out : Word,Word,Bit -> Word, Bit
#define ADDCIZO 3
  Opcode("addcizo",3,1),// Add with carry in, zero out  : Word,Word,Bit -> Word
#define SIGNEXT 4
  Opcode("signext",1,1),// Extend the input type onto the output type
};


static Projection* newValSplit(Block *source, Block *target)
{
Projection *p = new Projection();
  p->source  = source;
  p->target  = target;
  for(int i=0; i<source->numValues(); i++)
  {
    Value *v = source->getValue(i);
    if(v->type->kind == Type::UBITS && v->type->size > W)
    {
      printf("Non-native type @ %d: %s\n", i, v->type->repr().c_str());
      // Allocate machine words to cover the source value and build a mapping.
      std::vector<Value*> intervals;
      for(uint32 j=0; j<v->type->size; j+=W)
        intervals.push_back( p->target->value(Word) );
      p->mapping.push_back(intervals);

    }
    else
    {
      std::vector<Value*> intervals;
      intervals.push_back( target->value(v->type) );
      p->mapping.push_back( intervals );
    }
  }
  // Register the words making up each input as inputs to the new block.
  for(int i=0; i<source->numInputs(); i++)
  {
    Value *v = source->getInput(i);
    std::vector<Value*> inpWords = p->mapping[ v->ref ];
    for(int j=0; j<inpWords.size(); j++)
      target->input( inpWords[j] );
  }
  return p;
}

Projection *ArmMachine::translate(Block *block)
{
Block *target = new Block();  // TODO: Strange interface, could alloc inside next...
Projection *p = newValSplit(block,target);
  std::vector<Instruction*> order = block->topSort();
  printf("Trans: %zu insts\n",order.size());
  for(int i=0; i<order.size(); i++)
  {
    Instruction *inst = order[i];
    if(!strcmp(inst->opcode->name, "add"))
    {
      printf("Trans: ADD\n");
      std::vector<Value*> leftVals  = p->mapping[ inst->inputs[0]->ref ];
      std::vector<Value*> rightVals = p->mapping[ inst->inputs[1]->ref ];
      std::vector<Value*> targetVals = p->mapping[ inst->outputs[0]->ref ];

      // Build ADDCO from leftVals[i],rightVals[i] to targetVals[i]
      Instruction *newI = target->instruction(&opcodes[ADDCO]);
      newI->addInput(leftVals[0]);
      newI->addInput(rightVals[0]);
      newI->addOutput(targetVals[0]);
      Value *carry = newI->addOutput(Flag);

      for(int i=1; i<leftVals.size()-1; i++)
      {
        // Build ADDCICO from leftVals[i],rightVals[i] to targetVals[i]
        newI = target->instruction(&opcodes[ADDCICO]);
        newI->addInput(leftVals[i]);
        newI->addInput(rightVals[i]);
        newI->addInput(carry);
        newI->addOutput(targetVals[i]);
        carry = newI->addOutput(Flag);
      }
      // Decide how to handle the final carry flag
      if( targetVals.size() == leftVals.size() )
      {
        // Drop the final carry (no extra word to spill into)
        newI = target->instruction(&opcodes[ADDCIZO]);
        newI->addInput(leftVals[leftVals.size()-1]);
        newI->addInput(rightVals[rightVals.size()-1]);
        newI->addInput(carry);
        newI->addOutput(targetVals[targetVals.size()-1]);
      }
      else
      {
        // Spill the final carry into an extra word.
        newI = target->instruction(&opcodes[ADDCICO]);
        newI->addInput(leftVals[leftVals.size()-1]);
        newI->addInput(rightVals[rightVals.size()-1]);
        newI->addInput(carry);
        newI->addOutput(targetVals[targetVals.size()-2]);
        carry = newI->addOutput(Flag);
        newI = target->instruction(&opcodes[SIGNEXT]);
        newI->addInput(carry);
        newI->addOutput(targetVals[targetVals.size()-1]);
      }
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

bool trivial(Block *block, int numRegs, char const**regNames)
{

  return true;
}
