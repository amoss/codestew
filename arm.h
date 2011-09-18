#include "SimpleMachine.h"

class ArmMachine
{
public:
  Projection *translate(Block *block);
  std::string outCodeworks(Block *block);
  Allocation* allocate(Block *block);
};


