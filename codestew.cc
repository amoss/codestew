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
char temp[80];
  switch( kind )
  {
    case Type::UBITS:
      sprintf(temp,"Ubits<%llu>",size);
      return std::string(temp);  
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
  inp->uses.push_back(this);
}

Value *Instruction::addOutput( Type *type)
{
  Value *result = owner->value(type);
  result->def = this;
  outputs.push_back(result);
  return result;
}

Value *Instruction::addOutput( Value *val)
{
  val->def = this;
  outputs.push_back(val);
  return val;
}

std::string Instruction::repr()
{
char buffer[100];
  sprintf(buffer,"<Inst %llu in=%zu out=%zu>",ref,inputs.size(),outputs.size());
  return std::string(buffer);
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

void Block::input(Value *v)
{
  inputs.push_back(v->ref);
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

/* Using the owner container indices as refs inside the Value and Instruction objects
   allows a simple O(n) cloning operation for the graph. In the old CAO IR we had to
   build dictionaries to create the bijection of vertices/edges between the original
   and the clone. Here we can just split it into three passes:
     1. Create valid pointers (new objects) for each value ref
     2. Create valid pointers for each inst (mapping operands as defined in pass 1).
     3. Patch the pointers inside each value to the instructions defined in pass 2.
   Technically the clone is O(ve), but the degree of each instruction vertex is O(1).
*/
Block *Block::clone()
{
Block *result = new Block();
  copyInto(result);
  return result;
}

void Block::copyInto(Block *result)
{
  // Uses and def ptrs will be mapped in another pass.
  // Shallow sharing of types, assumes a small external set
  for(int i=0; i<values.size(); i++)
  {
    Value *v = new Value(values[i]->type);
    result->values.push_back(v); 
    v->ref = i;
  }

  // Shallow sharing of opcodes, assumes a small external set
  for(int i=0; i<insts.size(); i++)
  {
    Instruction *ins = new Instruction(result, insts[i]->opcode);
    result->insts.push_back(ins); 
    ins->ref = i;
    // Refs = vector indices = easy mapping once new ptrs are stored in previous loop
    for(int j=0; j<insts[i]->inputs.size(); j++)
      ins->addInput( result->values[ insts[i]->inputs[j]->ref ] );
    for(int j=0; j<insts[i]->outputs.size(); j++)
      ins->addOutput( result->values[ insts[i]->outputs[j]->ref ] );
  }

  // Patching of Instruction* within Value objects
  for(int i=0; i<values.size(); i++)
  {
    if( values[i]->def != NULL )  // Input have no defining instruction
      result->values[i]->def = result->insts[ values[i]->def->ref ];
    for(int j=0; j<values[i]->uses.size(); j++)
      result->values[i]->uses[j] = result->insts[ values[i]->uses[j]->ref ];
  }

  for(int i=0; i<inputs.size(); i++)
    result->inputs.push_back(inputs[i]);
  for(int i=0; i<outputs.size(); i++)
    result->outputs.push_back(outputs[i]);
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

/* Compute a topological sort of the bi-partite graph; compute a valid execution order that
   ensures every operand is computed before an instruction uses it. This is the obvious
   transative closure approach as that method wokred out faster in Python for the graphs
   tested than Tarjan's reverse-DFS algorithm. It might be worth implementing Tarjan's 
   algorithm for comparison as the operation complexities in STL probably follows a different
   trade-off and has different constant factors to the set implementation in Python.
*/
std::vector<Instruction*> Block::topSort()
{
std::vector<Instruction*> order;
std::set<uint64> ready;
std::set<Instruction*> done;
  printf("TOPSORT %zu %zu\n", insts.size(), values.size());
  std::copy(inputs.begin(), inputs.end(), inserter(ready,ready.begin()) );
  while(order.size() < insts.size())
  {
    //printf("Ready %d\n",ready.size());
    std::set<Instruction*> waiting;   // Instructions with at least one ready operand
    for(std::set<uint64>::iterator i = ready.begin(); i!=ready.end(); i++)
    {
      Value *v = values[*i];
      //printf("%llu has %d uses\n", v->ref, v->uses.size());
      for(int j=0; j<v->uses.size(); j++)
      {
        if(done.find(v->uses[j])==done.end())
          waiting.insert(v->uses[j]);
      }
    }
    //printf("Waiting %d\n",waiting.size());
    std::set<Instruction*> enabled;  // Instruction with all operands ready
    for(std::set<Instruction*>::iterator i = waiting.begin(); i!=waiting.end(); i++)
    {
      bool allReady=true;
      for(int j=0; j<(*i)->inputs.size(); j++)
        if( ready.find((*i)->inputs[j]->ref) == ready.end() ) {
          allReady=false;
          //printf("Value %llu not ready for %llu\n",(*i)->inputs[j]->ref,(*i)->ref);
        }
      if(allReady)
        enabled.insert(*i);
    }
    //printf("Enabled %d\n",enabled.size());
    if(enabled.size()==0)           // Graph is blocked (disjoint or cyclic), abort
    {
      printf("Graph is blocked: disjoint or cyclic\n");
      return order;
    }
    for(std::set<Instruction*>::iterator i=enabled.begin(); i!=enabled.end(); i++)
    {
      order.push_back(*i);
      done.insert(*i);
      for(int j=0; j<(*i)->outputs.size(); j++)
        ready.insert((*i)->outputs[j]->ref);
    }
  }
  printf("ORDER %zu\n",order.size());
  return order;
}

std::string Block::dump()
{
std::string result;
char line[180];
  for(int i=0; i<values.size(); i++)
  {
    Value *v = values[i];
    char const *annotation = "";
    if(isInput(v))
      annotation = "Input ";
    else if(isOutput(v))
      annotation = "Output ";
    if( v->defined() )
      sprintf(line,"Value %d: %s%s defined(%llu) %zu uses\n", i, annotation, 
             v->type->repr().c_str(), v->def->ref, v->uses.size());
    else
      sprintf(line,"Value %d: %s%s undefined %zu uses\n", i, annotation,
             v->type->repr().c_str(), v->uses.size());
    result += line;
  }
  for(int i=0; i<insts.size(); i++)
  {
    Instruction *inst = insts[i];
    sprintf(line,"Inst %d: [", i);
    for(int j=0; j<inst->outputs.size(); j++)
      sprintf(line+strlen(line),"%llu ",inst->outputs[j]->ref);
    sprintf(line+strlen(line),"] <- %s [", inst->opcode->name);
    for(int j=0; j<inst->inputs.size(); j++)
      sprintf(line+strlen(line),"%llu ",inst->inputs[j]->ref);
    sprintf(line+strlen(line),"]\n");
    result += line;
  }
  return result;
}

void Block::dot(const char *filename)
{
FILE *f = fopen(filename,"w");
  printf("Output to %s\n", filename);
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

/*char const *x86Names[] =
{
  "rax", "rbx", "rcx", "rdx", "rdi", "rsi", 
  "r8",  "r9",  "r10", "r11", "r12",  "r13",
  "r14", "r15"
};

void x86Output(Block *block)
{
  trivial(block, 14, x86Names);
}*/

std::string Projection::dump()
{
std::string res;
char line[120];
  for(int i=0; i<source->numValues(); i++)
  {
    sprintf(line, "Src %u -> Tar [", i);
    for(int j=0; j<mapping[i].size(); j++)
      sprintf(line+strlen(line), "%llu ", mapping[i][j]->ref);
    sprintf(line+strlen(line), "]\n");
    res += line;
  }
  return res;
}

Allocation::Allocation(Block *orig)
{
  orig->copyInto(this);
}

void Allocation::dot(const char *filename)
{
FILE *f = fopen(filename,"w");
  printf("Output to %s\n", filename);
  if(f==NULL)
    throw "Cannot open output file";
  fprintf(f,"digraph{\n");
  for(int i=0; i<values.size(); i++)
  {
    fprintf(f,"v%d [label=\"%s : %d : %s\"];\n", i, regs[i], i, values[i]->type->repr().c_str());
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
