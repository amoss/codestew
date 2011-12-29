#include "SimpleMachine.h"

class ArmMachine : public Machine
{
public:
  ArmMachine();
  Projection *translate(Block *block);
  std::string outCodeworks(Allocation *alloc);
  bool valid(Block *block);

  // Translation layer
  void translateUbitAdd(Instruction *inst, Projection *p);

  // Emit helper functions
  void   addzizo(Block *block, Value *l, Value *r, Value *out);
  Value* addzico(Block *block, Value *l, Value *r, Value *out);
  void   addcizo(Block *block, Value *l, Value *r, Value *car, Value *out);
  Value* addcico(Block *block, Value *l, Value *r, Value *car, Value *out);
  void   signext(Block *block, Value *in, Value *out);
};


