#include<stdio.h>
#include "arm.h"
#include "x86.h"

bool flag(const char *flag, int argc, char **argv)
{
  for(int i=1; i<argc; i++)
  {
    if(!strcmp(flag,argv[i]))
      return true;
  }
  return false;
}


int main(int argc, char **argv)
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
  block.dot((char*)"testcases/add128orig.dot");
  std::vector<Instruction*> order = block.topSort();
  printf("%zu\n",order.size());
  for(int i=0; i<order.size(); i++)
    printf("%llu\n",order[i]->ref);

  if(flag("--x86",argc,argv))
  {
    X86Machine x86;
    Projection *x86Proj = x86.translate(&block);
    printf("X86 translation:\n%s\nX86 block:\n%s\n",x86Proj->dump().c_str(),
                                                    x86Proj->target->dump().c_str());
    x86Proj->target->dot("testcases/add128x86.dot");
    Allocation *x86Regs = x86.allocate(x86Proj->target);
    x86Regs->dot("testcases/add128x86regs.dot");
    std::string x86Code = x86.outGccInline(x86Regs);
    printf("Code:\n%s\n", x86Code.c_str());
  }

  if(flag("--arm",argc,argv))
  {
    ArmMachine arm;
    Projection *armProj = arm.translate(&block);
    Block *armBlock = armProj->target;
    armBlock->dot("testcases/add128arm.dot");
    Allocation *armAlloc = arm.allocate(armBlock);
    armAlloc->dot((char*)"testcases/add128armregs.dot");
    printf("Code:\n%s\n", arm.outCodeworks(armBlock).c_str());
  }
}
