#include<stdio.h>
#include "SimpleMachine.h"

int main()
{
  printf("Hello\n");
  Block block;
  SimpleMachine machine;
  Type word = UBits(64);
  try {
    Value *in0 = block.input(&word);
    Value *in1 = block.input(&word);
    Value *t1  = machine.XOR( &block, in0, in1);  // Output type is implicit...
    printf("Hello\n");
    Value *t2  = machine.XOR( &block, in0, t1 ); 
  }
  catch(char const* err)
  {
    printf("ERROR: %s\n", err);
  }
}
