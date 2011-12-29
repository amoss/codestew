#include<stdio.h>
#include "arm.h"
#include "x86.h"

/* Deliberately memory-leak: we need the Type objects to be valid during compilation
   after we return control. Not very tidy, but not the end of the world either as we
   generally terminate after compilation.
*/

bool makeSource(Block &block, SimpleMachine &machine)
{
  Type *word512 = new Type(Type::UBITS, 512);
  Type *word513 = new Type(Type::UBITS, 513);
  try {
    Value *in0 = block.input(word512);
    Value *in1 = block.input(word512);
    Value *t1  = machine.ADD( &block, in0, in1, word513);
    block.output(t1);
  }
  catch(char const* err)
  {
    printf("ERROR: %s\n", err);
    return false;
  }
  return true;
}

