#include "SimpleMachine.h"

class X86Machine
{
public:
  void translate(X86Machine *source, Block *block);
  std::string outCodeworks();
};


