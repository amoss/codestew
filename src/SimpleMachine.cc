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

/* Currently this is the only definition of the semantics of the instruction-set
   for the SimpleMachine. This initial version is purely syntactic checking, it 
   needs to be complemented by an interpreter for the blocks.
*/
bool SimpleMachine::valid( Block *block )
{
  for(int i=0; i<block->numInsts(); i++)
  {
    Instruction *inst = block->getInst(i);
    if( inst->opcode == &opcodes[OP_ADD] ) {
      if( inst->inputs.size()!=2 || inst->outputs.size()!=1 ) {
        printf("Illegal add instruction for SimpleMachine\n");
        return false;
      }
      if( inst->inputs[0]->type->kind != Type::UBITS ||
          inst->inputs[1]->type->kind != Type::UBITS ) {
        printf("SimpleMachine::add is only defined over UBITS\n");
        return false;
      }
      if(!( inst->inputs[0]->type->size == inst->inputs[1]->type->size &&
           (inst->inputs[0]->type->size == inst->outputs[0]->type->size ||
            inst->inputs[0]->type->size == inst->outputs[0]->type->size-1) ))
      {
        printf("SimpleMachine::add ubits(%u) ubits(%u) -> ubits(%u) undefined\n");
        return false;
      }
    }
    if( inst->opcode == &opcodes[OP_XOR] ) {
      if( inst->inputs.size()!=2 || inst->outputs.size()!=1 ) {
        printf("Illegal xor instruction for SimpleMachine\n");
        return false;
      }
    }
  }
  return true;
}
