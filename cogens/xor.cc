#include<stdio.h>
#include "arm.h"

int main()
{
  Block block;
  SimpleMachine machine;
  Type word = Type(Type::UBITS, 64);
  try {
    Value *in0 = block.input(&word);
    Value *in1 = block.input(&word);
    Value *t1  = machine.XOR( &block, in0, in1);  // Output type is implicit...
    Value *t2  = machine.XOR( &block, in0, t1 ); 
    block.output(t2);
  }
  catch(char const* err)
  {
    printf("ERROR: %s\n", err);
  }
  std::string d = block.dump();
  printf("%s",d.c_str());
  block.dot((char*)"crap.dot");
  std::vector<Instruction*> order = block.topSort();
  printf("%d\n",order.size());
  for(int i=0; i<order.size(); i++)
    printf("%llu\n",order[i]->ref);
  //x86Output(&block);

  ArmMachine arm;
  Block *armBlock = arm.translate(&machine, &block);
  d = armBlock->dump();
  printf("%s",d.c_str());
  armBlock->dot((char*)"craparm.dot");
  arm.outCodeworks(armBlock);
}
