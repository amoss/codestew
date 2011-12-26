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

bool makeSource(Block &block, SimpleMachine &machine);

int main(int argc, char **argv)
{
  Block block;
  SimpleMachine machine;
  if(argc < 3) {
    printf("cogen: basename machine\n");
    return -1;
  }
  printf("Build source: %s\n", argv[1]);
  if(!makeSource(block,machine))
    return -1;

  if(!strcmp("src",argv[2])) {
    std::string newbase    = std::string(argv[1]) + "-orig";
    std::string newbaseDot = newbase + ".dot";
    std::string newbaseAsm = newbase + ".asm";
    FILE *out = fopen(newbaseAsm.c_str(),"wt");
    fprintf(out, block.dump().c_str() );
    fclose(out);
    printf("Output: %s\n", newbaseDot.c_str());
    block.dot(newbaseDot.c_str());
    return 0;
  }

  if(!strcmp("arm",argv[2])) {
    std::string newbase     = std::string(argv[1]) + "-arm";
    std::string withregs    = std::string(argv[1]) + "-arm-regs";
    std::string newbaseDot  = newbase  + ".dot";
    std::string withregsDot = withregs + ".dot";
    std::string newbaseAsm = newbase + ".asm";
    ArmMachine arm;
    Projection *armProj = arm.translate(&block);
    Block *armBlock = armProj->target;
    armBlock->dot(newbaseDot.c_str());
    Allocation *armAlloc = arm.allocate(armBlock);
    armAlloc->dot(withregsDot.c_str());
    FILE *out = fopen(newbaseAsm.c_str(),"wt");
    fprintf(out, "%s\n", arm.outCodeworks(armAlloc).c_str());
    fclose(out);
    return 0;
  }

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
}
