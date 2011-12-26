#include "SimpleMachine.h"

class ArmMachine : public Machine
{
public:
  ArmMachine();
  Projection *translate(Block *block);
  std::string outCodeworks(Allocation *alloc);
};


