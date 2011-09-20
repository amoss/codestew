#include "SimpleMachine.h"

class ArmMachine
{
public:
  Projection *translate(Block *block);
  std::string outCodeworks(Allocation *alloc);
  Allocation* allocate(Block *block);
};


