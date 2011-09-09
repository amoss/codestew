#include "SimpleMachine.h"

class X86Machine
{
public:
  Projection *translate(Block *block);
  Allocation *allocate(Block *block);
  std::string outGccInline(Allocation *p);
};


