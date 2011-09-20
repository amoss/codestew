#include <map>
#include <stdio.h>
#include <stdlib.h>
#include "arm.h"
#include "addProj.h"

#define W 32
static Type  *Word   = new Type(Type::UBITS,W);
static Type  *Flag   = new Type(Type::UBITS,1);


Projection *ArmMachine::translate(Block *block)
{
Projection *p = newValSplit(block,W);
  std::vector<Instruction*> order = block->topSort();
  printf("Trans: %zu insts\n",order.size());
  for(int i=0; i<order.size(); i++)
  {
    Instruction *inst = order[i];
    if(!strcmp(inst->opcode->name, "add"))
      translateUbitAdd(inst, p, (Machine*)this);
    else {
      printf("ERROR: Cannot translate\n");
      exit(-1);
    }
  }
  return p;
}

char const* regNames[] = {
  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9",
  "r10", "r11" // , "r12"
};
int numRegs = 12;
std::string ArmMachine::outCodeworks(Allocation *alloc)
{
  for(int i=0; i<alloc->numValues(); i++)
    printf("//Val %d: %s\n", i, alloc->regs[i]);
  for(int i=0; i<alloc->schedule.size(); i++)
  {
    if( !strcmp(alloc->schedule[i]->opcode->name, "addco") )
    {
      printf("  add %s,%s,%s\n",
             regNames[alloc->schedule[i]->inputs[0]->ref],
             regNames[alloc->schedule[i]->inputs[1]->ref],
             regNames[alloc->schedule[i]->outputs[0]->ref]);
    }
    else if( !strcmp(alloc->schedule[i]->opcode->name, "addcico") )
    {
      printf("  adc %s,%s,%s\n",
             regNames[alloc->schedule[i]->inputs[0]->ref],
             regNames[alloc->schedule[i]->inputs[1]->ref],
             regNames[alloc->schedule[i]->outputs[0]->ref]);
    }
    else if( !strcmp(alloc->schedule[i]->opcode->name, "signext") )
    {
      printf("  eor %s, %s, %s\n",
             alloc->regs[ alloc->schedule[i]->outputs[0]->ref ],
             alloc->regs[ alloc->schedule[i]->outputs[0]->ref ],
             alloc->regs[ alloc->schedule[i]->outputs[0]->ref ]);
      printf("  adc %s,%s,#0\n",
             alloc->regs[alloc->schedule[i]->inputs[0]->ref],
             alloc->regs[alloc->schedule[i]->outputs[0]->ref]);
    }
    else
    {
      printf("SLOT %d\n",i);
      printf("%s %s,%s,%s\n", alloc->schedule[i]->opcode->name, 
             regNames[alloc->schedule[i]->inputs[0]->ref],
             regNames[alloc->schedule[i]->inputs[1]->ref],
             regNames[alloc->schedule[i]->outputs[0]->ref]);
    }
  }
  return std::string();
}

static bool trivial(Allocation *regAlloc)
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

typedef std::map<Value*,char const*> LiveSet;
typedef std::set<char const*> FreePool;

static bool instReady(Instruction *inst, LiveSet live)
{
  for(int i=0; i<inst->inputs.size(); i++)
  {
    LiveSet::iterator it = live.find( inst->inputs[i] );
    if( it == live.end() )
      return false;
  }
  return true;
}

static void dumpState(LiveSet live, FreePool pool)
{
LiveSet::iterator lit;
FreePool::iterator pit;
  for(lit=live.begin(); lit!=live.end(); lit++)
      printf("%s:%s ", (*lit).first->type->repr().c_str(), (*lit).second);
  printf("| ");
  for(pit=pool.begin(); pit!=pool.end(); pit++)
    printf("%s ", *pit);
  printf("\n");
}

static bool checkKill(Instruction *inst, Value *v, std::set<Instruction*> done)
{
  for(int i=0; i<v->uses.size(); i++)
  {
    if( done.find(v->uses[i]) == done.end() )
      return false;
  }
  return true;
}

static bool allocReg(Value *v, LiveSet &live, FreePool &pool, Allocation *al)
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

static bool modulo(Allocation *regAlloc)
{
LiveSet live;
FreePool pool;
std::set<Instruction*> done;
  for(int i=0; i<regAlloc->numInputs(); i++)
  {
    live[regAlloc->getInput(i)] = regNames[i];
    regAlloc->regs[regAlloc->getInput(i)->ref] = regNames[i];
  }
  for(int i=regAlloc->numInputs(); i<numRegs; i++)
    pool.insert(regNames[i]);

  while(done.size() < regAlloc->schedule.size() )
  {
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

Allocation *ArmMachine::allocate(Block *block)
{
Allocation *regAlloc = new Allocation(block);
  regAlloc->schedule = regAlloc->topSort();
  // Presize / initial state is NULL 
  for(int i=0; i<block->numValues(); i++)
    regAlloc->regs.push_back(NULL);

  // Nail up fixed registers based on instruction types
  for(int i=0; i<block->numInsts(); i++)
  {
    Instruction *inst = block->getInst(i);
    if( !strcmp(inst->opcode->name,"addco") || !strcmp(inst->opcode->name,"addcico")) {
      regAlloc->regs[ inst->outputs[1]->ref ] = "carry";
    }
  }

  if( trivial(regAlloc) )
    return regAlloc; 
  if( modulo(regAlloc) )
    return regAlloc;
  printf("Modulo failed\n");
  exit(-1);
  return NULL;
}
