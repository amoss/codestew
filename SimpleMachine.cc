#include "SimpleMachine.h"

/* These objects are intentionally singleton objects. Their addresses are used as
   ids to check for equality...
*/
static Opcode opcodes[] =
{
#define OP_XOR 0
  Opcode("xor",2,1)
};

Type *SimpleMachine::ubits(int n)
{
}

Value *SimpleMachine::XOR( Block *block, Value *in0, Value *in1)
{
  Instruction *inst = block->instruction( &opcodes[OP_XOR] );
  if( !in0->type->equals(in1->type) )
    throw "XOR on inequal value types";
  inst->addInput( in0 );
  inst->addInput( in1 );
  Value *result = inst->addOutput( in0->type );
  return result;
}
