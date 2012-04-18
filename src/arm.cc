#include <map>
#include <stdio.h>
#include <stdlib.h>
#include "arm.h"
#include "addProj.h"

#define W 32
static Type  *Word   = new Type(Type::UBITS,W);
static Type  *Flag   = new Type(Type::UBITS,1);

static Opcode armOpcodes[] =
{
  Opcode("addzico",2,2),  // Add with carry out :           Word,Word -> Word, Bit
#define OP_ADDZICO 0
  Opcode("addzizo",2,1),  // Add with zero out :            Word,Word -> Word
#define OP_ADDZIZO 1
  Opcode("addcico",3,2),// Add with carry in, carry out : Word,Word,Bit -> Word, Bit
#define OP_ADDCICO 2
  Opcode("addcizo",3,1),// Add with carry in, zero out  : Word,Word,Bit -> Word
#define OP_ADDCIZO 3
  Opcode("signext",1,1),// Extend the input type onto the output type
#define OP_SIGNEXT 4
  Opcode("xor",2,1),    // Word, Word -> Word
#define OP_XOR 5
  Opcode("mul",2,1),    // Word, Word -> 2xWord
#define OP_MUL 6
  Opcode(NULL,-1,-1)
};

Projection *ArmMachine::translate(Block *block)
{
Projection *p = newValSplit(block,W,*this);
  std::vector<Instruction*> order = block->topSort();
  printf("Trans: %zu insts\n",order.size());
  for(int i=0; i<order.size(); i++)
  {
    Instruction *inst = order[i];
    if(!strcmp(inst->opcode->name, "add"))
      translateUbitAdd(inst, p);
    else if(!strcmp(inst->opcode->name, "xor"))
      translateUbitXor(inst, p);
    else if(!strcmp(inst->opcode->name, "mul"))
      translateUbitMul(inst, p);
    else {
      printf("ERROR: Cannot translate\n");
      exit(-1);
    }
  }
  return p;
}

/* Translate an addition over Ubits<n> to a series of additions over the W-bit
   words stored in the Projection.
*/
void ArmMachine::translateUbitAdd(Instruction *inst, Projection *p)
{
  std::vector<Value*> leftVals  = p->mapping[ inst->inputs[0]->ref ];
  std::vector<Value*> rightVals = p->mapping[ inst->inputs[1]->ref ];
  std::vector<Value*> targetVals = p->mapping[ inst->outputs[0]->ref ];
  printf("Trans: ADD %zu %zu -> %zu\n", leftVals.size(), rightVals.size(), 
                                        targetVals.size());
  // ASSERT(from prev valid): leftVals.size() == rightVals.size()
  size_t n = leftVals.size();
  bool inCarry = false;
  // For all but the final word assume that a carry is produced within the chain.
  Instruction *newI;
  Value *carry;

  if( n==1 && targetVals.size()==1 )  // Single-word, drop carry
    addzizo( p->target, leftVals[0],   rightVals[0],   targetVals[0] );
  else if( n==1 )                     // Single-word, spill carry
  {
    carry = addzico( p->target, leftVals[n-1], rightVals[n-1], targetVals[n-1] );   
    signext( p->target, carry, targetVals[n] );
  }
  else 
  {
    carry = addzico( p->target,   leftVals[0], rightVals[0],        targetVals[0] );
    for(int i=1; i<n-1; i++)
      carry = addcico( p->target, leftVals[i], rightVals[i], carry, targetVals[i] );

    if( targetVals.size()==n )    // Drop final carry
      addcizo( p->target, leftVals[n-1], rightVals[n-1], carry, targetVals[n-1] );
    else                          // Spill the final carry into an extra word
    {
      carry = addcico( p->target, leftVals[n-1], rightVals[n-1], carry, targetVals[n-1]);
      signext( p->target, carry, targetVals[n] );
    }
  }
}

void ArmMachine::translateUbitMul(Instruction *inst, Projection *p)
{
  
}

void ArmMachine::translateUbitXor(Instruction *inst, Projection *p)
{
  std::vector<Value*> leftVals  = p->mapping[ inst->inputs[0]->ref ];
  std::vector<Value*> rightVals = p->mapping[ inst->inputs[1]->ref ];
  std::vector<Value*> targetVals = p->mapping[ inst->outputs[0]->ref ];
  printf("Trans: XOR %zu %zu -> %zu\n", leftVals.size(), rightVals.size(), 
                                        targetVals.size());
  // ASSERT(from prev valid): leftVals.size() == rightVals.size()
  size_t n = leftVals.size();
  for(int i=0; i<n; i++)
    emitXor( p->target, leftVals[i], rightVals[i], targetVals[i] );
}

void   ArmMachine::addzizo(Block *block, Value *l, Value *r, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_ADDZIZO] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addOutput(out);
}

Value* ArmMachine::addzico(Block *block, Value *l, Value *r, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_ADDZICO] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addOutput(out);
  return inst->addOutput(Flag);
}

void   ArmMachine::addcizo(Block *block, Value *l, Value *r, Value *car, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_ADDCIZO] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addInput(car); 
  inst->addOutput(out);
}

Value* ArmMachine::addcico(Block *block, Value *l, Value *r, Value *car, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_ADDCICO] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addInput(car); 
  inst->addOutput(out);
  return inst->addOutput(Flag);
}

void   ArmMachine::signext(Block *block, Value *in, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_SIGNEXT] );
  inst->addInput(in); 
  inst->addOutput(out);
}

void   ArmMachine::emitXor(Block *block, Value *l, Value *r, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_XOR] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addOutput(out);
}

static char const* armRegNames[] = {
  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9",
  "r10", "r11" // , "r12"
};

ArmMachine::ArmMachine()
{
  this->numRegs = 12;
  this->regNames = armRegNames;
}

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

bool ArmMachine::valid( Block *block )
{
  return false;
}
