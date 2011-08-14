#include<stdio.h>
#include "arm.h"

int main()
{
  Block block;
  SimpleMachine machine;
  Type word128 = Type(Type::UBITS, 128);
  Type word129 = Type(Type::UBITS, 129);
  try {
    Value *in0 = block.input(&word128);
    Value *in1 = block.input(&word128);
    Value *t1  = machine.ADD( &block, in0, in1, &word129);
    block.output(t1);
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
