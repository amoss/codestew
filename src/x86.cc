#include <stdio.h>
#include <stdlib.h>
#include "x86.h"
#include "addProj.h"




#define W 64

/*struct Interval
{
  Value *source;
  uint32 lo, size;
  Value *target;
}; */

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
  Opcode(NULL,-1,-1)
};


Projection *X86Machine::translate(Block *block)
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
void X86Machine::translateUbitAdd(Instruction *inst, Projection *p)
{
Type  *Flag   = new Type(Type::UBITS,1);
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

void X86Machine::translateUbitXor(Instruction *inst, Projection *p)
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

void   X86Machine::addzizo(Block *block, Value *l, Value *r, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_ADDZIZO] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addOutput(out);
}

Value* X86Machine::addzico(Block *block, Value *l, Value *r, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_ADDZICO] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addOutput(out);
  return inst->addOutput(Flag);
}

void   X86Machine::addcizo(Block *block, Value *l, Value *r, Value *car, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_ADDCIZO] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addInput(car); 
  inst->addOutput(out);
}

Value* X86Machine::addcico(Block *block, Value *l, Value *r, Value *car, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_ADDCICO] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addInput(car); 
  inst->addOutput(out);
  return inst->addOutput(Flag);
}

void   X86Machine::signext(Block *block, Value *in, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_SIGNEXT] );
  inst->addInput(in); 
  inst->addOutput(out);
}

void   X86Machine::emitXor(Block *block, Value *l, Value *r, Value *out)
{
Instruction *inst = block->instruction( &armOpcodes[OP_XOR] );
  inst->addInput(l); 
  inst->addInput(r); 
  inst->addOutput(out);
}


static char const *x86RegNames[] =
{
  "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14"
};
#define NUMREGS (sizeof(regNames)/sizeof(char const *))

X86Machine::X86Machine()
{
  this->numRegs = NUMREGS;
  this->regNames = x86RegNames;
}

static bool opCoalesces(Opcode *op)
{
  return !strcmp("addco",op->name)   || !strcmp("addzo",op->name) ||
         !strcmp("addcico",op->name) || !strcmp("addcizo",op->name) ;
}

void X86Machine::preAllocActions(Allocation *alloc)
{
  // Check for coalesced regs first as reduces reg-pressure
  for(int i=0; i<alloc->numValues(); i++)
  {
    Value *v = alloc->getValue(i);
    if(v->defined() && opCoalesces(v->def->opcode) && v==v->def->outputs[0])
      alloc->regs[i] = "implied";
  }
}

std::string X86Machine::outGccInline(Allocation *alloc)
{
std::string result;
char line[120];
  result += "__asm__ __volatile__(\"\\\n";
  for(int i=0; i<alloc->regs.size(); i++)
  {
    sprintf(line, "// REGMAP %d : %s\\\n", i, alloc->regs[i]);
    result += line;
  }
  std::vector<Instruction*> order = alloc->topSort();
  printf("SIZE %zu\n",order.size());
  for(int i=0; i<order.size(); i++)
  {
    if( !strcmp(order[i]->opcode->name, "addco") ) {
      sprintf(line,"  addq %%%s, %%%s;\n", alloc->regs[ order[i]->inputs[0]->ref ], 
                                       alloc->regs[ order[i]->inputs[1]->ref ]);
      result += line;
    }
    else if( !strcmp(order[i]->opcode->name, "addcico") ) {
      sprintf(line,"  adcq %%%s, %%%s;\n", alloc->regs[ order[i]->inputs[0]->ref ], 
                                       alloc->regs[ order[i]->inputs[1]->ref ]);
      result += line;
    }
    else if( !strcmp(order[i]->opcode->name, "signext") &&
             !strcmp("carry",alloc->regs[order[i]->inputs[0]->ref]) ) {
      const char *regName = alloc->regs[ order[i]->outputs[0]->ref ];
      sprintf(line,"  xorq %%%s, %%%s;\n  adc #0, %%%s\n",
                   regName, regName, regName);
      result += line;
    }
    else {
      sprintf(line,"OTHER %s\n",order[i]->opcode->name);
      result += line;
    }
    
  }
  result += "\");\n";
  return result;
}

bool X86Machine::valid(Block *block)
{
  return false;
}
