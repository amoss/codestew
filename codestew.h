#ifndef CODESTEW_H
#define CODESTEW_H
#include<vector>
#include<string>
#include<set>
#include<iterator>
#include<string.h>

typedef unsigned long long uint64 ;
typedef unsigned long uint32 ;

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
  Value *addOutput( Value *val);
  std::string repr();
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
protected:
  std::vector<Instruction*> insts;
  std::vector<Value*> values;
  std::vector<uint64> inputs, outputs;

public:
  Value *value(Type *type);
  Instruction *instruction(Opcode *opcode);
  Value *input(Type *type);
  void input(Value *v);
  void output(Value *v);

  Block *clone();
  void copyInto(Block *result);

  bool isInput(Value *v);
  bool isOutput(Value *v);
  size_t numInputs() { return inputs.size(); }
  Value* getInput(int index) { return values[inputs[index]]; }
  size_t numValues() { return values.size(); }
  Value* getValue(int index) { return values[index]; }
  size_t numInsts()  { return insts.size(); }
  Instruction *getInst(int index) { return insts[index]; }
  std::vector<Instruction*> topSort();

  std::string dump();
  void dot(char *filename);
};

/* Store a mapping between values in the left block and values in the right block.
   This mapping may not be injective or surjective. Normally there will be a 
   correspondence between input/output values stored in a Projection. One value
   in the left block may be split into several values in the right.
*/
class Projection
{
public:
  Block *source, *target;
  std::vector< std::vector<Value*> > mapping;
  std::string dump();
};

class Allocation : public Block
{
public:
std::vector< char const * > regs;
  Allocation(Block *orig);
  void dot(char *filename);

};

class Machine
{
  
};

//void x86Output(Block *block);
#endif
