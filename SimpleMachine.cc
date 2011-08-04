#include "SimpleMachine.h"

const char *SimpleMachine::opcodeNames[OP_MAX] =
{
  "xor"
};

uint64 SimpleMachine::opcode(char *name)
{
  for(uint64 i=0; i<OP_MAX; i++)
    if( !strcmp(name,opcodeNames[i]) )
      return i;
  throw "Invalid opcode name";
}

Type *SimpleMachine::ubits(int n)
{
}

Value *SimpleMachine::XOR( Block *block, Value *in0, Value *in1)
{
  Instruction *inst = block->instruction( OP_XOR );
  if( !in0->type->equals(in1->type) )
    throw "XOR on inequal value types";
  inst->addInput( in0 );
  inst->addInput( in1 );
  Value *result = inst->addOutput( in0->type );
  return result;
}
