#include "codestew.h"
#include <string.h>

enum {
  OP_XOR,
  OP_MAX
};
class SimpleMachine : public Machine
{
  static const char *opcodeNames[OP_MAX];
public:

  uint64 opcode(char *name);
  Type *ubits(int n);
  Value *XOR(Block *block, Value *in0, Value *in1);
};
