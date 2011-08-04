#include<stdio.h>
#include "codestew.h"


UBits::UBits(uint64 size)
{
  this->size = size;
}

bool UBits::equals(UBits *other)
{
  printf("Compare UBits\n");
  return other->size == size;
}

bool Type::equals(Type *other)
{
  return false;
}

Instruction::Instruction(Block *owner, uint64 opcode)
{
  this->opcode = opcode;
  this->owner  = owner;
}

void Instruction::addInput( Value *inp)
{
  inputs.push_back(inp);
}

Value *Instruction::addOutput( Type *type)
{
  Value *result = owner->value(type);
  outputs.push_back(result);
  return result;
}

Value::Value(Type *type)
{
  this->type = type;
}

Value *Block::input(Type *type)
{
Value *result = value(type);
  inputs.push_back(result->ref);
  return result;
}

Value *Block::value(Type *type)
{
Value *result = new Value(type);
  values.push_back(result);
  return result;
}

Instruction *Block::instruction(uint64 opcode)
{
Instruction *result = new Instruction(this,opcode);
  result->ref = insts.size();
  insts.push_back(result);
  return result;
}
