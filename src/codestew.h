#ifndef CODESTEW_H
#define CODESTEW_H
#include<vector>
#include<string>
#include<set>
#include<map>
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
  Type( Kind, uint64 size=0 );
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

class UbitsConstant
{
public:
  UbitsConstant(int n) {
    size = n;
    bits = new bool[n];
  }
  int size;
  bool *bits;
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
  void *constant;
};

class Machine;
class Block
{
protected:
  std::vector<Instruction*> insts;
  std::vector<Value*> values;
  std::vector<uint64> inputs, outputs;
  Machine &machine;

public:
  Block( Machine &m ) : machine(m) {}
  Value *value(Type *type);
  Value *constant(Type *type, uint64 init);
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
  size_t numOutputs() { return outputs.size(); }
  Value* getOutput(int index) { return values[outputs[index]]; }
  size_t numValues() { return values.size(); }
  Value* getValue(int index) { return values[index]; }
  size_t numInsts()  { return insts.size(); }
  Instruction *getInst(int index) { return insts[index]; }
  std::vector<Instruction*> topSort();

  std::string dump();
  void dot(const char *filename);

  bool valid();
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
std::vector< Instruction* > schedule;
  Allocation(Block *orig);
  void dot(const char *filename);

};

typedef std::map<Value*,char const*> LiveSet;
typedef std::set<char const*> FreePool;

class Machine
{
public:
  int numRegs;
  char const **regNames;
  Machine() {} 
  Opcode *lookup(const char *name);  
  virtual void preAllocActions(Allocation *alloc) {}
  Allocation *allocate(Block *block);
  bool trivial(Allocation *regAlloc);
  bool instReady(Instruction *inst, LiveSet live);
  void dumpState(LiveSet live, FreePool pool);
  bool checkKill(Instruction *inst, Value *v, std::set<Instruction*> done);
  bool allocReg(Value *v, LiveSet &live, FreePool &pool, Allocation *al);
  bool modulo(Allocation *regAlloc);
  virtual bool valid(Block *block) = 0;
};

//void x86Output(Block *block);
#endif
