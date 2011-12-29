#include "SimpleMachine.h"

class X86Machine : public Machine
{
public:
  X86Machine();
  Projection *translate(Block *block);
  void preAllocActions(Allocation *alloc);
  std::string outGccInline(Allocation *p);
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


