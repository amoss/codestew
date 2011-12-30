#include<stdio.h>
#include "arm.h"
#include "x86.h"

/* Deliberately memory-leak: we need the Type objects to be valid during compilation
   after we return control. Not very tidy, but not the end of the world either as we
   generally terminate after compilation.
*/

bool makeSource(Block &block, SimpleMachine &machine)
{
  Type *single = new Type(Type::UBITS, 64);
  try {
    Value *in0 = block.input(single);
    Value *in1 = block.input(single);
    Value *t1  = machine.MUL( &block, in0, in1);
    block.output(t1);
  }
  catch(char const* err)
  {
    printf("ERROR: %s\n", err);
    return false;
  }
  return true;
}

