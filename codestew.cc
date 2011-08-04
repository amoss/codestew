#include<stdio.h>
#include "codestew.h"

Type::Type(Kind kind, uint64 y)
{
  this->kind = kind;
  size = y;
}

bool Type::equals(Type *other)
{
  return (kind==other->kind) && (size==other->size);
}

std::string Type::repr()
{
  switch( kind )
  {
    case Type::UBITS:
      return std::string("Ubits");  
    default:
      return std::string("UNKNOWN");
  }
}

Opcode::Opcode(char const*name, int inputs, int output)
{
  this->name = name;
  this->inputs = inputs;
  this->outputs = outputs;
}

Instruction::Instruction(Block *owner, Opcode *opcode)
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
  result->def = this;
  outputs.push_back(result);
  return result;
}

Value::Value(Type *type)
{
  this->type = type;
  def = NULL;
}

Value *Block::input(Type *type)
{
Value *result = value(type);
  inputs.push_back(result->ref);
  return result;
}

void Block::output(Value *v)
{
  outputs.push_back(v->ref);
}

Value *Block::value(Type *type)
{
Value *result = new Value(type);
  result->ref = values.size();
  values.push_back(result);
  return result;
}

Instruction *Block::instruction(Opcode *opcode)
{
Instruction *result = new Instruction(this,opcode);
  result->ref = insts.size();
  insts.push_back(result);
  return result;
}

bool Block::isInput(Value *v)
{
  for(int i=0; i<inputs.size(); i++)
    if(v->ref == inputs[i])
      return true;
  return false;
}

bool Block::isOutput(Value *v)
{
  for(int i=0; i<outputs.size(); i++)
    if(v->ref == outputs[i])
      return true;
  return false;
}

std::vector<Instruction*> Block::topSort()
{
std::vector<Instruction*> order;
  return order;
}

void Block::dump()
{
  for(int i=0; i<values.size(); i++)
  {
    Value *v = values[i];
    char const *annotation = "";
    if(isInput(v))
      annotation = "Input ";
    else if(isOutput(v))
      annotation = "Output ";
    if( v->defined() )
      printf("Value %d: %s%s defined(%llu) %zu uses\n", i, annotation, 
             v->type->repr().c_str(), v->def->ref, v->uses.size());
    else
      printf("Value %d: %s%s undefined %zu uses\n", i, annotation,
             v->type->repr().c_str(), v->uses.size());
  }
  for(int i=0; i<insts.size(); i++)
  {
    Instruction *inst = insts[i];
    printf("Inst %d: [", i);
    for(int j=0; j<inst->outputs.size(); j++)
      printf("%llu ",inst->outputs[j]->ref);
    printf("] <- %s [", inst->opcode->name);
    for(int j=0; j<inst->inputs.size(); j++)
      printf("%llu ",inst->inputs[j]->ref);
    printf("]\n");
  }
}

void Block::dot(char *filename)
{
FILE *f = fopen(filename,"w");
  if(f==NULL)
    throw "Cannot open output file";
  fprintf(f,"digraph{\n");
  for(int i=0; i<values.size(); i++)
  {
    fprintf(f,"v%d [label=\"%d : %s\"];\n", i, i, values[i]->type->repr().c_str());
  }
  for(int i=0; i<insts.size(); i++)
  {
    Instruction *inst = insts[i];
    fprintf(f,"i%d [shape=none,label=\"%s\"];\n", i, inst->opcode->name);
    for(int j=0; j<inst->inputs.size(); j++)
      fprintf(f,"v%llu -> i%d;\n", inst->inputs[j]->ref, i);
    for(int j=0; j<inst->outputs.size(); j++)
      fprintf(f,"i%d -> v%llu;\n", i, inst->outputs[j]->ref);
  }
  fprintf(f,"}");
  fclose(f);
}
