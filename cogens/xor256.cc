#include<stdio.h>
#include "arm.h"
#include "x86.h"

/* Deliberately memory-leak: we need the Type objects to be valid during compilation
   after we return control. Not very tidy, but not the end of the world either as we
   generally terminate after compilation.
*/

bool makeSource(Block &block, SimpleMachine &machine)
{
  Type *word256 = new Type(Type::UBITS, 256);
  try {
    Value *in0 = block.input(word256);
    Value *in1 = block.input(word256);
    Value *t1  = machine.LXOR( &block, in0, in1);
    block.output(t1);
  }
  catch(char const* err)
  {
    printf("ERROR: %s\n", err);
    return false;
  }
  return true;
}

