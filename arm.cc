#include "arm.h"

Block *ArmMachine::translate(SimpleMachine *source, Block *block)
{
  // Most basic translate is just a deep-copy of the block, no attempt at filtering
  // or converting yet
Block *result = block->clone();
  return result;
}

std::string ArmMachine::outCodeworks()
{
}
