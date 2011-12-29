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
