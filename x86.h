#include "SimpleMachine.h"

class X86Machine
{
public:
  Block *translate(Block *block);
  std::string outGccInline();
};


