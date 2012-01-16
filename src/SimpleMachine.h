#ifndef SIMPLEMACHINE_H
#define SIMPLEMACHINE_H

#include "codestew.h"
#include <string.h>
#include <stdio.h>

class SimpleMachine : public Machine
{
public:
  SimpleMachine() {}
  uint64 opcode(char *name);
  Type *ubits(int n);
  Value *LXOR(Block *block, Value *in0, Value *in1);
  Value *LAND(Block *block, Value *in0, Value *in1);
  Value *LNOT(Block *block, Value *in);
  Value *LOR(Block *block,  Value *in0, Value *in1);

  Value *COPY(Block *block, Value *in);
  Value *COPY(Block *block, Value *in, Value *out);

  Value *ADD(Block *block, Value *in0, Value *in1, Type *resType);
  Value *MUL(Block *block, Value *in0, Value *in1);

  Value *CONCAT(Block *block, Value *lowBits, Value *hiBits);
  Value *EXTRACT(Block *block, Value *src, int lo, int hi);

  Value *UPROT(Block *block, Value *in, int num);

  


  bool valid(Block *block);
};
#endif
