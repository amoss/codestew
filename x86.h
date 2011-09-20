#include "SimpleMachine.h"

class X86Machine : public Machine
{
public:
  X86Machine();
  Projection *translate(Block *block);
  void preAllocActions(Allocation *alloc);
  std::string outGccInline(Allocation *p);
};


