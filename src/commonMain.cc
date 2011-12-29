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
  SimpleMachine machine;
  Block block(machine);
  if(argc < 3) {
    printf("cogen: basename machine\n");
    return -1;
  }
  printf("Build source: %s\n", argv[1]);
  if(!makeSource(block,machine) || !block.valid()) {
    printf("Failure\n");
    return -1;
  }

  if(!strcmp("src",argv[2])) {
    std::string newbase    = std::string(argv[1]) + "-orig";
    std::string newbaseDot = newbase + ".dot";
    std::string newbaseAsm = newbase + ".asm";
    FILE *out = fopen(newbaseAsm.c_str(),"wt");
    fprintf(out, "%s", block.dump().c_str() );
    fclose(out);
    block.dot(newbaseDot.c_str());
    printf("Success\n");
    return 0;
  }

  if(!strcmp("arm",argv[2])) {
    std::string newbase     = std::string(argv[1]) + "-arm";
    std::string withregs    = std::string(argv[1]) + "-arm-regs";
    std::string newbaseDot  = newbase  + ".dot";
    std::string withregsDot = withregs + ".dot";
    std::string newbaseAsm  = newbase + ".asm";
    ArmMachine arm;
    Projection *armProj = arm.translate(&block);
    if( armProj==NULL ) {
      printf("Failure\n");
      return -1;
    }
    Block *armBlock = armProj->target;
    armBlock->dot(newbaseDot.c_str());
    Allocation *armAlloc = arm.allocate(armBlock);
    if( armAlloc==NULL ) {
      printf("Failure\n");
      return -1;
    }
    armAlloc->dot(withregsDot.c_str());
    FILE *out = fopen(newbaseAsm.c_str(),"wt");
    fprintf(out, "%s\n", arm.outCodeworks(armAlloc).c_str());
    fclose(out);
    printf("Success\n");
    return 0;
  }

  if(!strcmp("x86",argv[2])) {
    std::string newbase     = std::string(argv[1]) + "-x86";
    std::string withregs    = std::string(argv[1]) + "-x86-regs";
    std::string newbaseDot  = newbase  + ".dot";
    std::string withregsDot = withregs + ".dot";
    std::string newbaseAsm  = newbase + ".asm";
    X86Machine x86;
    Projection *x86Proj = x86.translate(&block);
    if( x86Proj==NULL ) {
      printf("Failure\n");
      return -1;
    }
    x86Proj->target->dot(newbaseDot.c_str());
    Allocation *x86Regs = x86.allocate(x86Proj->target);
    if( x86Regs==NULL ) {
      printf("Failure\n");
      return -1;
    }
    x86Regs->dot(withregsDot.c_str());
    FILE *out = fopen(newbaseAsm.c_str(), "wt");
    fprintf(out, "X86 translation:\n%s\nX86 block:\n%s\n",
                 x86Proj->dump().c_str(), x86Proj->target->dump().c_str());
    std::string x86Code = x86.outGccInline(x86Regs);
    fprintf(out, "Code:\n%s\n", x86Code.c_str());
    fclose(out);
    printf("Success\n");
    return 0;
  }

  printf("Unknown target architecture: %s\n", argv[2]);
  return -1;
}
