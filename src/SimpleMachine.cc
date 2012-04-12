#include "SimpleMachine.h"

/* These objects are intentionally singleton objects. Their addresses are used as
   ids to check for equality...
*/
static Opcode opcodes[] =
{
#define OP_XOR 0
  Opcode("xor",2,1),
#define OP_ADD 1
  Opcode("add",2,1),
#define OP_MUL 2
  Opcode("mul",2,1),
#define OP_COPY 3
  Opcode("copy",1,1),
#define OP_LAND 4
  Opcode("land",2,1),
#define OP_LNOT 5
  Opcode("lnot",1,1),
#define OP_LOR 5
  Opcode("lor",2,1),
#define OP_EXTRACT 6
  Opcode("extract",3,1),
#define OP_UPROT 7
  Opcode("uprot",2,1),
#define OP_CONCAT 8
  Opcode("concat",2,1)
};

Type *SimpleMachine::ubits(int n)
{
}

Value *SimpleMachine::LXOR( Block *block, Value *in0, Value *in1)
{
Instruction *inst = block->instruction( &opcodes[OP_XOR] );
  if( !in0->type->equals(in1->type) )
    throw "XOR on inequal value types";
  inst->addInput( in0 );
  inst->addInput( in1 );
  Value *result = inst->addOutput( in0->type );
  return result;
}

Value *SimpleMachine::LAND(Block *block, Value *in0, Value *in1)
{
Instruction *inst = block->instruction( &opcodes[OP_LAND] );
  if( !in0->type->equals(in1->type) )
    throw "LAND on inequal value types";
  inst->addInput( in0 );
  inst->addInput( in1 );
  Value *result = inst->addOutput( in0->type );
  return result;
}

Value *SimpleMachine::LNOT(Block *block, Value *in)
{
Instruction *inst = block->instruction( &opcodes[OP_LNOT] );
  inst->addInput( in );
  Value *result = inst->addOutput( in->type );
  return result;
}

Value *SimpleMachine::LOR(Block *block,  Value *in0, Value *in1)
{
Instruction *inst = block->instruction( &opcodes[OP_LOR] );
  if( !in0->type->equals(in1->type) )
    throw "LOR on inequal value types";
  inst->addInput( in0 );
  inst->addInput( in1 );
  Value *result = inst->addOutput( in0->type );
  return result;
}

Value *SimpleMachine::COPY(Block *block, Value *in)
{
Instruction *inst = block->instruction( &opcodes[OP_COPY] );
  inst->addInput( in );
  Value *result = inst->addOutput( in->type );
  return result;
}

// NOTE: Can't remember how we're handling carries at this point...
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


Value *SimpleMachine::MUL(Block *block, Value *in0, Value *in1)
{
  Instruction *inst = block->instruction( &opcodes[OP_MUL] );
  printf("%u %u\n", in0->type->kind, in1->type->kind );
  if( in0->type->kind!=Type::UBITS ||
      in1->type->kind!=Type::UBITS )
    throw "MUL on non-UBITS value types";
  // TODO: Standard memory leak on type object
  Type *resType = new Type( Type::UBITS, in0->type->size+in1->type->size );
  inst->addInput( in0 );
  inst->addInput( in1 );
  Value *result = inst->addOutput( resType );
  return result;
}

Value *SimpleMachine::CONCAT(Block *block, Value *lowBits, Value *hiBits)
{
  Instruction *inst = block->instruction( &opcodes[OP_CONCAT] );
  if( lowBits->type->kind!=Type::UBITS ||
      hiBits->type->kind!=Type::UBITS )
    throw "CONCAT on non-UBITS value types";
  // TODO: Standard memory leak on type object
  Type *resType = new Type( Type::UBITS, lowBits->type->size+hiBits->type->size );
  inst->addInput( lowBits );
  inst->addInput( hiBits );
  Value *result = inst->addOutput( resType );
  return result;
}

Value *SimpleMachine::EXTRACT(Block *block, Value *src, int lo, int hi)
{
  if(src->type->kind != Type::UBITS)
    throw "EXTRACT not yet implemented on non-UBITS type";
Instruction *inst = block->instruction( &opcodes[OP_EXTRACT] );
  inst->addInput(src);
Type *ints = new Type( Type::INT );
Value *loParam = block->constant( ints, (uint64)lo );
Value *hiParam = block->constant( ints, (uint64)hi );
  inst->addInput(loParam);
  inst->addInput(hiParam);
int size = hi-lo+1;
Type *resType = new Type( Type::UBITS, size);
Value *result = inst->addOutput(resType);
  return result;
}

Value *SimpleMachine::UPROT(Block *block, Value *src, int num)
{
  if(src->type->kind != Type::UBITS)
    throw "UPROT not yet implemented on non-UBITS type";
Instruction *inst = block->instruction( &opcodes[OP_UPROT] );
  inst->addInput(src);
Type *ints = new Type( Type::INT );
Value *shift = block->constant( ints, (uint64)num );
  inst->addInput(shift);
Value *result = inst->addOutput(src->type);
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
