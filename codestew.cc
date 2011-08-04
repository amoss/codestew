#include "codestew.h"

UBits::UBits(uint64 size)
{
  this->size = size;
}

bool Type::equals(Type *other)
{
  return false;
}

Instruction::Instruction(Block *owner, uint64 opcode)
{
  this->opcode = opcode;
  this->owner  = owner;
  numInputs  = 0;
  numOutputs = 0;
  inputsMax   = 4;
  outputsMax  = 4;
  inputs  = new Value*[inputsMax ];
  outputs = new Value*[outputsMax];
}

void Instruction::addInput( Value *inp)
{
  if( numInputs==inputsMax )
  {
    uint64 n = inputsMax*2;
    Value **newInputs = new Value*[n];
    for(int i=0; i<inputsMax; i++)
      newInputs[i] = inputs[i];
    delete[] inputs;
    inputs = newInputs;
    inputsMax = n;
  }
  inputs[numInputs++] = inp;
}

Value *Instruction::addOutput( Type *type)
{
  if( numOutputs==outputsMax )
  {
    uint64 n = outputsMax*2;
    Value **newOutputs = new Value*[n];
    for(int i=0; i<outputsMax; i++)
      newOutputs[i] = outputs[i];
    delete[] outputs;
    outputs = newOutputs;
    outputsMax = n;
  }
  Value *result = owner->value(type);
  outputs[numOutputs++] = result;
  return result;
}

Value::Value(Type *type)
{
  this->type = type;
  numUses  = 0;
  numDefs  = 0;
  usesMax  = 4;
  defsMax  = 4;
  uses = new Instruction*[usesMax];
  defs = new Instruction*[defsMax];
}

Value *Block::input(Type *type)
{
  /*if( numValues==valuesMax )
  {
    uint64 n = valuesMax*2;
    Value **newValues = new Value*[n];
    for(int i=0; i<valuesMax; i++)
      newValues[i] = values[i];
    delete[] values;
    values = newValues;
    valuesMax = n;
  }*/
}

Value *Block::value(Type *type)
{
  if( numValues==valuesMax )
  {
    uint64 n = valuesMax*2;
    Value **newValues = new Value*[n];
    for(int i=0; i<valuesMax; i++)
      newValues[i] = values[i];
    delete[] values;
    values = newValues;
    valuesMax = n;
  }
Value *result = new Value(type);
  result->ref = numValues;
  values[numValues++] = result;
  return result;
}

Instruction *Block::instruction(uint64 opcode)
{
  if( numInsts==instsMax )
  {
    uint64 n = instsMax*2;
    Instruction **newInsts = new Instruction*[n];
    for(int i=0; i<instsMax; i++)
      newInsts[i] = insts[i];
    delete[] insts;
    insts = newInsts;
    instsMax = n;
  }
Instruction *result = new Instruction(this,opcode);
  result->ref = numInsts;
  insts[numInsts++] = result;
  return result;
}
