#include <stdio.h>
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

static Type  *Word   = new Type(Type::UBITS,W);
static Type  *Flag   = new Type(Type::UBITS,1);

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
  return p;
}

Projection *X86Machine::translate(Block *block)
{
Block *target = new Block;
Projection *p = newValSplit(block,target);

  std::vector<Instruction*> order = block->topSort();
  printf("Trans: %d insts\n",order.size());
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

static char const *regNames[] =
{
  "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14"
};
#define NUMREGS (sizeof(regNames)/sizeof(char const *))

static bool opCoalesces(Opcode *op)
{
  return op==&opcodes[ADDCO]   || op==&opcodes[ADDZO] ||
         op==&opcodes[ADDCICO] || op==&opcodes[ADDCIZO];
}

std::string X86Machine::outGccInline(Projection *p)
{
std::string result;
char line[120];
std::vector< char const * > regAlloc;
  for(int i=0; i<p->target->numValues(); i++)
    regAlloc.push_back(NULL);
  result += "__asm__ __volatile__(\"\\\n";
  for(int i=0; i<p->target->numValues(); i++)
  {
    Value *v = p->target->getValue(i);
    if(v->defined() && opCoalesces(v->def->opcode) && v==v->def->outputs[0])
      regAlloc[i] = "implied";
  }
  int regsNeeded = 0;
  for(int i=0; i<regAlloc.size(); i++)
    if( regAlloc[i]==NULL )
      regsNeeded++;
  if(regsNeeded <= NUMREGS)
  {
    int regCounter=0;
    for(int i=0; i<regAlloc.size(); i++)
      if(regAlloc[i]==NULL)
        regAlloc[i] = regNames[regCounter++];
    for(int i=0; i<regAlloc.size(); i++)
    {
      if(!strcmp(regAlloc[i], "implied"))
      {
        printf("Coalescing %u\n",i);
        Value *v = p->target->getValue(i);
        Instruction *inst = v->def;
        printf("From %u\n",inst->ref);
        Value *overwritten = inst->inputs[1];
        regAlloc[i] = regAlloc[overwritten->ref];
      }
    }
    for(int i=0; i<regAlloc.size(); i++)
    {
      sprintf(line, "// REGMAP %d : %s\\\n", i, regAlloc[i]);
      result += line;
    }
  }
  else {
    result += "Trivial failed\n";
  }
  result += "\");\n";
  return result;
}
