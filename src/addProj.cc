#include <stdio.h>
#include "addProj.h"

/* Prepare a block for projection onto the lower-level machine $m$. Split every
   Ubits<n> where n>W for some register size W into a set of W-bit words.
*/
Projection* newValSplit(Block *source, int W, Machine &m)
{
Projection *p = new Projection();
Type  *Word   = new Type(Type::UBITS,W);
  p->source  = source;
  p->target  = new Block(m);
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
      intervals.push_back( p->target->value(v->type) );
      p->mapping.push_back( intervals );
    }
  }
  // Register the words making up each input as inputs to the new block.
  for(int i=0; i<source->numInputs(); i++)
  {
    Value *v = source->getInput(i);
    std::vector<Value*> inpWords = p->mapping[ v->ref ];
    for(int j=0; j<inpWords.size(); j++)
      p->target->input( inpWords[j] );
  }
  // Ditto for outputs
  for(int i=0; i<source->numOutputs(); i++)
  {
    Value *v = source->getOutput(i);
    std::vector<Value*> opWords = p->mapping[ v->ref ];
    for(int j=0; j<opWords.size(); j++)
      p->target->output( opWords[j] );
  }
  return p;
}

/* Translate an addition over Ubits<n> to a series of additions over the W-bit
   words stored in the Projection.
*/
void translateUbitAdd(Instruction *inst, Projection *p, Machine *m)
{
Type  *Flag   = new Type(Type::UBITS,1);
  std::vector<Value*> leftVals  = p->mapping[ inst->inputs[0]->ref ];
  std::vector<Value*> rightVals = p->mapping[ inst->inputs[1]->ref ];
  std::vector<Value*> targetVals = p->mapping[ inst->outputs[0]->ref ];
  printf("Trans: ADD %u %u -> %u\n", leftVals.size(), rightVals.size(), 
                                     targetVals.size());

  bool inCarry = false;
  // For all but the final word assume that a carry is produced within the chain.
  Instruction *newI;
  Value *carry;
  for(int i=0; i<leftVals.size()-1; i++)
  {
    if( inCarry )
      newI = p->target->instruction(m->lookup("addcico"));
    else 
      newI = p->target->instruction(m->lookup("addco"));
    newI->addInput(leftVals[i]);
    newI->addInput(rightVals[i]);
    if( inCarry )
      newI->addInput(carry);
    newI->addOutput(targetVals[i]);
    carry = newI->addOutput(Flag);
    inCarry = true;
  }
  if( targetVals.size() == leftVals.size() )
  {
    // Drop the final carry (no extra word to spill into)
    if( inCarry )
      newI = p->target->instruction(m->lookup("addcizo"));
    else
      newI = p->target->instruction(m->lookup("add"));   // ?? Perhaps call this addzizo
    newI->addInput(leftVals[leftVals.size()-1]);
    newI->addInput(rightVals[rightVals.size()-1]);
    if( inCarry )
      newI->addInput(carry);
    newI->addOutput(targetVals[targetVals.size()-1]);
  }
  else
  {
    // Spill the final carry into an extra word.
    if( inCarry )
      newI = p->target->instruction(m->lookup("addcico"));
    else
      newI = p->target->instruction(m->lookup("addco"));   // ?? Perhaps call this addzico

    newI->addInput(leftVals[leftVals.size()-1]);
    newI->addInput(rightVals[rightVals.size()-1]);
    if( inCarry )
      newI->addInput(carry);
    newI->addOutput(targetVals[targetVals.size()-2]);
    carry = newI->addOutput(Flag);
    newI = p->target->instruction(m->lookup("signext"));
    newI->addInput(carry);
    newI->addOutput(targetVals[targetVals.size()-1]);
  }
}
