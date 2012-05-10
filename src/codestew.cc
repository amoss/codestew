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
    case Type::INT:
      return std::string("Int");
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
  sprintf(buffer,"<Inst %llu %s in=%zu out=%zu>",ref,opcode->name, inputs.size(),outputs.size());
  return std::string(buffer);
}

Value::Value(Type *type)
{
  this->type = type;
  def = NULL;
  this->constant = NULL;
}

std::string Value::repr()
{
char result[128];
  if(constant!=NULL)
    switch(type->kind)
    {
      case Type::UBITS:
        sprintf(result, "v%u [..] : %s", ref, type->repr().c_str());
        break;
      case Type::INT:
        sprintf(result, "v%u #%d : %s", ref, ((IntConstant*)constant)->value, type->repr().c_str());
        break;
    }
  else
    sprintf(result, "v%u : %s", ref, type->repr().c_str());
  return std::string(result);
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

Value *Block::constant(Type *type, uint64 init)
{
Value *v;
  switch(type->kind)
  {
    case Type::UBITS:
      v = value(type);
      v->constant = new UbitsConstant(type->size);
      return v;
    case Type::INT:
      v = value(type);
      v->constant = new IntConstant((int)init);
      return v;
    default:
      throw "Block::constant not yet implemented";
  }
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
Block *result = new Block(this->machine);
  copyInto(result);
  return result;
}

/* Performs a deepcopy, cloning Values/Instructions and then updating refs.
   Destroys any previous data in result 
*/
void Block::copyInto(Block *result)
{
  result->values.erase( result->values.begin(), result->values.end() );
  result->insts.erase( result->insts.begin(), result->insts.end() );
  result->inputs.erase( result->inputs.begin(), result->inputs.end() );
  result->outputs.erase( result->outputs.begin(), result->outputs.end() );
  

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

  result->machine = machine;
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
  //printf("TOPSORT %zu %zu\n", insts.size(), values.size());
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
      printf("Graph is blocked: disjoint or cyclic, completed %zu/%zu\n",
             order.size(), insts.size());
      for(int i=0; i<order.size(); i++)
        printf("Order %d: %llu\n", i, order[i]->ref);
      for(std::set<Instruction*>::iterator i = waiting.begin(); i!=waiting.end(); i++)
        printf("Waiting: %llu\n", (*i)->ref);
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
    // Tolerate broken instructions to allow intermediate states in graph mutations
    for(int j=0; j<inst->inputs.size(); j++)
      if(inst->inputs[j]==NULL)
        sprintf(line+strlen(line),"emp ");
      else
        sprintf(line+strlen(line),"%llu ",inst->inputs[j]->ref);
    sprintf(line+strlen(line),"]\n");
    result += line;
  }
  return result;
}

void Block::dot(const char *filename)
{
FILE *f = fopen(filename,"w");
  if(f==NULL)
    throw "Cannot open output file";
  fprintf(f,"digraph{\n");
  fprintf(f,"input [shape=none];\noutput [shape=none];\n");
  for(int i=0; i<values.size(); i++)
  {
    fprintf(f,"v%d [label=\"%d : %s\"];\n", i, i, values[i]->type->repr().c_str());
  }
  for(int i=0; i<inputs.size(); i++)
    fprintf(f,"input -> v%llu [color=grey];\n", inputs[i]);
  for(int i=0; i<outputs.size(); i++)
    fprintf(f,"v%llu -> output [color=grey];\n", outputs[i]);
  for(int i=0; i<insts.size(); i++)
  {
    Instruction *inst = insts[i];
    fprintf(f,"i%d [shape=none,label=\"%s\"];\n", i, inst->opcode->name);
    // Tolerate broken instructions so that we can view intermediate states during graph mutation
    for(int j=0; j<inst->inputs.size(); j++)
      if(inst->inputs[j]!=NULL)
        fprintf(f,"v%llu -> i%d;\n", inst->inputs[j]->ref, i);
    for(int j=0; j<inst->outputs.size(); j++)
      fprintf(f,"i%d -> v%llu;\n", i, inst->outputs[j]->ref);
  }
  fprintf(f,"}");
  fclose(f);
}

bool Block::valid()
{
  return this->machine.valid(this);
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
  : Block(*orig) // Dummy to init ref, all overwritten by copyInto()
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
  fprintf(f,"input [shape=none];\noutput [shape=none];\n");
  for(int i=0; i<values.size(); i++)
  {
    fprintf(f,"v%d [label=\"%s : %d : %s\"];\n", i, regs[i], i, values[i]->type->repr().c_str());
  }
  for(int i=0; i<inputs.size(); i++)
    fprintf(f,"input -> v%llu [color=grey];\n", inputs[i]);
  for(int i=0; i<outputs.size(); i++)
    fprintf(f,"v%llu -> output [color=grey];\n", outputs[i]);
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

/*static Opcode genericOps[] =
{
  Opcode("addco",2,2),  // Add with carry out :           Word,Word -> Word, Bit
  Opcode("addzo",2,1),  // Add with zero out :            Word,Word -> Word
  Opcode("addcico",3,2),// Add with carry in, carry out : Word,Word,Bit -> Word, Bit
  Opcode("addcizo",3,1),// Add with carry in, zero out  : Word,Word,Bit -> Word
  Opcode("signext",1,1),// Extend the input type onto the output type
  Opcode(NULL,-1,-1)
};

Opcode *Machine::lookup(const char *name)
{
  for(int i=0; genericOps[i].name!=NULL; i++)
    if(!strcmp(name,genericOps[i].name) )
      return &genericOps[i];
  return NULL;
}*/

bool Machine::trivial(Allocation *regAlloc)
{
  // Check if value set is small enough for trivial allocation
  int regsNeeded = 0;
  for(int i=0; i<regAlloc->regs.size(); i++)
    if( regAlloc->regs[i]==NULL )
      regsNeeded++;
  if(regsNeeded <= numRegs)
  {
    int regCounter=0;
    for(int i=0; i<regAlloc->regs.size(); i++)
      if(regAlloc->regs[i]==NULL)
        regAlloc->regs[i] = regNames[regCounter++];
    return true;
  }
  printf("Trivial failed %d/%d\n",regsNeeded,numRegs);
  return false;
}

bool Machine::instReady(Instruction *inst, LiveSet live)
{
  for(int i=0; i<inst->inputs.size(); i++)
  {
    LiveSet::iterator it = live.find( inst->inputs[i] );
    if( it == live.end() )
      return false;
  }
  return true;
}

void Machine::dumpState(LiveSet live, FreePool pool)
{
LiveSet::iterator lit;
FreePool::iterator pit;
  for(lit=live.begin(); lit!=live.end(); lit++)
  {
    //printf("%llu %llu\n", (*lit).first, (*lit).second);
    printf("%s:%s ", (*lit).first->type->repr().c_str(), (*lit).second);
  }
  printf("| ");
  for(pit=pool.begin(); pit!=pool.end(); pit++)
    printf("%s ", *pit);
  printf("\n");
}

bool Machine::checkKill(Instruction *inst, Value *v, std::set<Instruction*> done)
{
  for(int i=0; i<v->uses.size(); i++)
  {
    if( done.find(v->uses[i]) == done.end() )
      return false;
  }
  return true;
}

bool Machine::allocReg(Value *v, LiveSet &live, FreePool &pool, Allocation *al)
{
FreePool::iterator it;
  if(pool.size()==0)
    return false;
  if( al->regs[v->ref]==NULL )
  {
    it = pool.begin();
    live[v] = *it;
    al->regs[v->ref] = *it;
    pool.erase(it);
  }
  else
  {
    printf("Skipping %llu, preallocated to %s\n", v->ref, al->regs[v->ref] );
    live[v] = al->regs[v->ref];
  }
  return true;
}

bool Machine::modulo(Allocation *regAlloc)
{
LiveSet live;
FreePool pool;
std::set<Instruction*> done;
  if( regAlloc->numInputs() > numRegs )
  {
    printf("Too many inputs (%zu) for registers (%u), ABI impossible!\n", 
           regAlloc->numInputs(), numRegs);
    return false;
  }
  for(int i=0; i<regAlloc->numInputs(); i++)
  {
    live[regAlloc->getInput(i)] = regNames[i];
    regAlloc->regs[regAlloc->getInput(i)->ref] = regNames[i];
  }
  for(int i=regAlloc->numInputs(); i<numRegs; i++)
    pool.insert(regNames[i]);

  while(done.size() < regAlloc->schedule.size() )
  {
    printf("Modulo pass: %zu / %zu\n", done.size(), regAlloc->schedule.size() );
    dumpState(live,pool);
    for(int i=0; i<regAlloc->schedule.size(); i++)
    {
      Instruction *inst = regAlloc->schedule[i];
      std::set<Instruction*>::iterator it=done.find(inst);
      if( it!=done.end() )
        continue;
      if( instReady(inst,live) )
      {
        // Atomic assumption: work out which registers are consumed by this instruction
        // (this is the last use in this ordering) and free those registers before 
        // allocating targets.
        printf("Proc %s ready\n", inst->repr().c_str());
        done.insert(inst);
        for(int i=0; i<inst->inputs.size(); i++)
          if( checkKill(inst, inst->inputs[i], done) )
          {
            LiveSet::iterator it = live.find(inst->inputs[i]);
            char const *reg = (*it).second;
            printf("Killed %llu, freed %s\n", inst->inputs[i]->ref, reg);
            live.erase(it); 
            pool.insert( reg );
          }
          else
            printf("Still live %llu\n", inst->inputs[i]->ref);
        for(int i=0; i<inst->outputs.size(); i++)
        {
          if(!allocReg(inst->outputs[i], live, pool, regAlloc))
          {
            printf("Can't allocate for %s (%zu)\n", inst->repr().c_str(), pool.size());
            return false;
          }
        }
      }
      else
        printf("Proc %s waiting\n", inst->repr().c_str());
    }
  }
  dumpState(live,pool);

  return true;
}

Allocation *Machine::allocate(Block *block)
{
// Allocation is w.r.t. a particular instruction schedule
Allocation *regAlloc = new Allocation(block);
  regAlloc->schedule = regAlloc->topSort();

  // Presize / initial state is NULL 
  for(int i=0; i<block->numValues(); i++)
    regAlloc->regs.push_back(NULL);

  // Nail up fixed registers based on (generic) instruction types
  for(int i=0; i<block->numInsts(); i++)
  {
    Instruction *inst = block->getInst(i);
    if( !strcmp(inst->opcode->name,"addco") || !strcmp(inst->opcode->name,"addcico")) {
      regAlloc->regs[ inst->outputs[1]->ref ] = "carry";
    }
  }

  // Let the specific machine handle coalescing / specific instruction types
  preAllocActions(regAlloc);

  // Find an allocator that works
  if( trivial(regAlloc) )
    return regAlloc; 
  if( modulo(regAlloc) )
    return regAlloc;

  printf("All allocators failed\n");
  return NULL;
}
