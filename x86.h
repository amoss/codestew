#include "SimpleMachine.h"

class X86Machine
{
public:
  Projection *translate(Block *block);
  std::string outGccInline(Projection *p);
};


