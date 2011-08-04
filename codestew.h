#include<vector>
#include<string>
typedef unsigned long long uint64 ;

class Type {
public:
  enum Kind {
    UBITS, SBITS, INT, VECTOR, MATRIX, CUSTOM
  } kind;
  uint64 size;
  Type( Kind, uint64 );
  bool equals(Type *other);
  std::string repr();
};

class Opcode
{
public:
  char const *name;
  int inputs, outputs;
  Opcode(char const*name, int inputs, int output);
};

class Value;
class Block;
class Instruction
{
  Block *owner;
public:
  std::vector<Value*> inputs, outputs;
  uint64 ref;
  Opcode *opcode;

  Instruction(Block *owner, Opcode *opcode);
  void addInput( Value *inp);
  Value *addOutput( Type *type);
};

class Value
{
public:
  std::vector<Instruction*> uses;
  Instruction *def;
  uint64 ref;
  Type *type;
  Value(Type *type);
  bool defined() { return def!=NULL; }
};


class Block
{
  std::vector<Instruction*> insts;
  std::vector<Value*> values;
  std::vector<uint64> inputs, outputs;

public:
  Value *value(Type *type);
  Instruction *instruction(Opcode *opcode);
  Value *input(Type *type);
  void output(Value *v);
  bool isInput(Value *v);
  bool isOutput(Value *v);
  void dump();
};


class Machine
{
  
};

