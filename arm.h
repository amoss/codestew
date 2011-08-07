#include "SimpleMachine.h"

class ArmMachine
{
public:
  Block *translate(SimpleMachine *source, Block *block);
  std::string outCodeworks(Block *block);
};


