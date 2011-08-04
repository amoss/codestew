#include<vector>
typedef unsigned long long uint64 ;
class Type {
public:
  virtual bool equals(Type *other);

};

class UBits : public Type
{
public:
  uint64 size;
  UBits(uint64 size);
  bool equals(UBits *other);
};

class Value;
class Block;
class Instruction
{
  std::vector<Value*> inputs, outputs;
  Block *owner;
public:
  uint64 opcode, ref;

  Instruction(Block *owner, uint64 opcode);
  void addInput( Value *inp);
  Value *addOutput( Type *type);
};

class Value
{
  std::vector<Instruction*> defs, uses;
public:
  uint64 ref;
  Type *type;
  Value(Type *type);
};


class Block
{
  std::vector<Instruction*> insts;
  std::vector<Value*> values;
  std::vector<uint64> inputs, outputs;

public:
  Value *value(Type *type);
  Instruction *instruction(uint64 opcode);
  Value *input(Type *type);
};


class Machine
{
  
};

