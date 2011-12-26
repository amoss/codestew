#include "SimpleMachine.h"

/* These objects are intentionally singleton objects. Their addresses are used as
   ids to check for equality...
*/
static Opcode opcodes[] =
{
#define OP_XOR 0
  Opcode("xor",2,1),
#define OP_ADD 1
  Opcode("add",2,1)
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

Value *SimpleMachine::ADD( Block *block, Value *in0, Value *in1, Type *resType)
{
  Instruction *inst = block->instruction( &opcodes[OP_ADD] );
  if( !in0->type->equals(in1->type) )
    throw "ADD on inequal value types";
  inst->addInput( in0 );
  inst->addInput( in1 );
  Value *result = inst->addOutput( resType );
  return result;
}
