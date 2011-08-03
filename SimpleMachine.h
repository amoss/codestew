#include "codestew.h"

class SimpleMachine : public Machine
{
  const char *opcodeNames[] =
  {
    "xor",
    NULL
  };

  uint64 opcode(char *name)
  {
    for(uint64 i=0; opcodeNames[i]!=NULL; i++)
      if( !strcmp(name,opcodeNames[i]) )
        return i;
    throw "Invalid opcode name";
  }

  Value *xor( Block *block, Value *in0, Value *in1)
  {
    Instruction *inst = block.instruction( XOR );
    if( !in0->type.equals(in1->type) )
      throw "XOR on inequal value types";
    inst->addInput( in0 );
    inst->addInput( in1 );
    Value *result = block.value( in0->type )
    inst->addOutput( result );
    return result;
  }
};
