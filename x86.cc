#include "x86.h"




#define W 64

struct Interval
{
  Value *source;
  uint32 lo, size;
  Value *target;
};

static Opcode opcodes[] =
{
#define ADDCO 0
  Opcode("add",2,2),    // Add with carry out :           Word,Word -> Word, Bit
#define ADDZO 1
  Opcode("add",2,1),    // Add with zero out :            Word,Word -> Word
#define ADDCICO 2
  Opcode("addcico",3,2),// Add with carry in, carry out : Word,Word,Bit -> Word, Bit
#define ADDCIZO 3
  Opcode("addcizo",3,1),// Add with carry in, zero out  : Word,Word,Bit -> Word
};

static Type  *Word   = new Type(Type::UBITS,W);
static Type  *Flag   = new Type(Type::UBITS,1);

Block *X86Machine::translate(Block *block)
{
Block *result = new Block;
Projection p;
std::vector< std::vector<Value*> > mapping;
  for(int i=0; i<block->numValues(); i++)
  {
    Value *v = block->getValue(i);
    if(v->type->kind == Type::UBITS && v->type->size > W)
    {
      printf("Non-native type: %s\n", v->type->repr().c_str());
      // Allocate machine words to cover the source value and build a mapping.
      std::vector<Value*> intervals;
      for(uint32 j=0; j<v->type->size; j+=W)
        intervals.push_back( result->value(Word) );
      mapping.push_back(intervals);

    }
    else
    {
      std::vector<Value*> intervals;
      intervals.push_back( result->value(v->type) );
      mapping.push_back( intervals );
    }
  }
  std::vector<Instruction*> order = block->topSort();
  printf("Trans: %d insts\n",order.size());
  for(int i=0; i<order.size(); i++)
  {
    Instruction *inst = order[i];
    if(!strcmp(inst->opcode->name, "add"))
    {
      printf("Trans: ADD\n");
      std::vector<Value*> leftVals  = mapping[ inst->inputs[0]->ref ];
      std::vector<Value*> rightVals = mapping[ inst->inputs[1]->ref ];
      std::vector<Value*> targetVals = mapping[ inst->outputs[0]->ref ];

      // Build ADDCO from leftVals[i],rightVals[i] to targetVals[i]
      Instruction *newI = result->instruction(&opcodes[ADDCO]);
      newI->addInput(leftVals[0]);
      newI->addInput(rightVals[0]);
      newI->addOutput(targetVals[0]);
      Value *carry = newI->addOutput(Flag);

      for(int i=1; i<leftVals.size()-1; i++)
      {
        // Build ADDCICO from leftVals[i],rightVals[i] to targetVals[i]
        newI = result->instruction(&opcodes[ADDCICO]);
        newI->addInput(leftVals[i]);
        newI->addInput(rightVals[i]);
        newI->addInput(carry);
        newI->addOutput(targetVals[i]);
        carry = newI->addOutput(Flag);
      }
      // Build ADDCI for final word
      newI = result->instruction(&opcodes[ADDCIZO]);
      newI->addInput(leftVals[leftVals.size()-1]);
      newI->addInput(rightVals[rightVals.size()-1]);
      newI->addInput(carry);
      newI->addOutput(targetVals[targetVals.size()-1]);
    }
  }
  return result;
}

std::string X86Machine::outGccInline()
{
}
