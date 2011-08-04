typedef unsigned long long uint64 ;
class Type {
public:
  bool equals(Type *other);

};

class UBits : public Type
{
public:
  uint64 size;
  UBits(uint64 size);
};

class Value;
class Block;
class Instruction
{
  uint64 numInputs, inputsMax, numOutputs, outputsMax;
  Value **inputs, **outputs;
  Block *owner;
public:
  uint64 opcode, ref;

  Instruction(Block *owner, uint64 opcode);
  void addInput( Value *inp);
  Value *addOutput( Type *type);
};

class Value
{
  uint64 numUses, usesMax, numDefs, defsMax;
  Instruction **defs;
  Instruction **uses;
public:
  uint64 ref;
  Type *type;
  Value(Type *type);
};


class Block
{
  uint64 numInsts, instsMax, numValues, valuesMax;
  Instruction **insts;
  Value **values;

public:
  Value *value(Type *type);
  Instruction *instruction(uint64 opcode);
  Value *input(Type *type);
};


class Machine
{
  
};

