#include<stdio.h>
#include "arm.h"
#include "x86.h"

/* Deliberately memory-leak: we need the Type objects to be valid during compilation
   after we return control. Not very tidy, but not the end of the world either as we
   generally terminate after compilation.
*/

bool makeSource(Block &block, SimpleMachine &machine)
{
  Type *word64 = new Type(Type::UBITS, 64);
  Type *word65 = new Type(Type::UBITS, 65);
  try {
    Value *in0 = block.input(word64);
    Value *in1 = block.input(word64);
    Value *t1  = machine.ADD( &block, in0, in1, word65);
    block.output(t1);
  }
  catch(char const* err)
  {
    printf("ERROR: %s\n", err);
    return false;
  }
  return true;
}

