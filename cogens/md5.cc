#include<stdio.h>
#include<math.h>
#include "arm.h"
#include "x86.h"

/* Deliberately memory-leak: we need the Type objects to be valid during compilation
   after we return control. Not very tidy, but not the end of the world either as we
   generally terminate after compilation.
*/

bool makeSource(Block &block, SimpleMachine &machine)
{
try {
  Type *Word  = new Type(Type::UBITS, 32);
  Type *Pair  = new Type(Type::UBITS, 64);
  Type *Chunk = new Type(Type::UBITS, 512);
  Type *Hash  = new Type(Type::UBITS, 128);

  Value *in = block.input(Chunk);

  Value *iv0 = block.constant(Word, 0x67452301);
  Value *iv1 = block.constant(Word, 0xefcdab89);
  Value *iv2 = block.constant(Word, 0x98badcfe);
  Value *iv3 = block.constant(Word, 0x10325476);


  Value *k[64];
  for(int i=0; i<64; i++)
    k[i] = block.constant(Word, (unsigned int)floor(abs(sin(i+1.0)) * pow(2,32)));
  int r[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
               5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
               4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
               6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
  

  Value *h0 = machine.COPY(&block, iv0);
  Value *h1 = machine.COPY(&block, iv1);
  Value *h2 = machine.COPY(&block, iv2);
  Value *h3 = machine.COPY(&block, iv3);
  Value *a = machine.COPY(&block, h0);
  Value *b = machine.COPY(&block, h1);
  Value *c = machine.COPY(&block, h2);
  Value *d = machine.COPY(&block, h3);

  for(int i=0; i<16; i++)
  {
    Value *t1 =    machine.LAND(&block, b,    c            );
    Value *t2 =    machine.LNOT(&block, b                  );
    Value *t3 =    machine.LAND(&block, t2,   d            );
    Value *f  =     machine.LOR(&block, t1,   t3           );
    Value *t4 =     machine.ADD(&block, a,    f,    Word   );
    Value *t5 = machine.EXTRACT(&block, in,   i*32, i*32+31);
    Value *t6 =     machine.ADD(&block, k[i], t5,   Word   );
    Value *t7 =     machine.ADD(&block, t6,   t4,   Word   );
    Value *t8 =   machine.UPROT(&block, t7,   r[i]         );
    Value *t9 =     machine.ADD(&block, b,    t8,   Word   );   // b'
    Value *t10 = d;
    d = c;
    c = b;
    b = t9;
    a = t10;
  }

  for(int i=16; i<32; i++)
  {
    Value *t1 =    machine.LAND(&block, d,    b            );
    Value *t2 =    machine.LNOT(&block, d                  );
    Value *t3 =    machine.LAND(&block, t2,   c            );
    Value *f  =     machine.LOR(&block, t1,   t3           );
    Value *t4 =     machine.ADD(&block, a,    f,    Word   );
    int base = (((i*5)+1)%16) * 32;
    Value *t5 = machine.EXTRACT(&block, in,   base, base+31);
    Value *t6 =     machine.ADD(&block, k[i], t5,   Word   );
    Value *t7 =     machine.ADD(&block, t6,   t4,   Word   );
    Value *t8 =   machine.UPROT(&block, t7,   r[i]         );
    Value *t9 =     machine.ADD(&block, b,    t8,   Word   );   // b'
    Value *t10 = d;
    d = c;
    c = b;
    b = t9;
    a = t10;
  }

  for(int i=32; i<48; i++)
  {
    Value *t1 =    machine.LXOR(&block, b,    c            );
    Value *f  =    machine.LXOR(&block, t1,   d            );
    Value *t2 =     machine.ADD(&block, a,    f,    Word   );
    int base = (((i*3)+5)%16) * 32;
    Value *t3 = machine.EXTRACT(&block, in,   base, base+31);
    Value *t4 =     machine.ADD(&block, k[i], t3,   Word   );
    Value *t5 =     machine.ADD(&block, t2,   t4,   Word   );
    Value *t6 =   machine.UPROT(&block, t5,   r[i]         );
    Value *t7 =     machine.ADD(&block, b,    t6,   Word   );   // b'
    Value *t8 = d;
    d = c;
    c = b;
    b = t7;
    a = t8;
  }

  for(int i=48; i<64; i++)
  {
    Value *t1 =    machine.LNOT(&block, d                  );
    Value *t2 =     machine.LOR(&block, t1,    b            );
    Value *f  =    machine.LXOR(&block, c,    t2           );
    Value *t3 =     machine.ADD(&block, a,    f,    Word   );
    int base = (((i*7)+1)%16) * 32;
    Value *t4 = machine.EXTRACT(&block, in,   base, base+31);
    Value *t5 =     machine.ADD(&block, k[i], t4,   Word   );
    Value *t6 =     machine.ADD(&block, t3,   t5,   Word   );
    Value *t7 =   machine.UPROT(&block, t6,   r[i]         );
    Value *t8 =     machine.ADD(&block, b,    t7,   Word   );   // b'
    Value *t9 = d;
    d = c;
    c = b;
    b = t8;
    a = t9;
  }

  h0 = machine.ADD(&block,  h0, a, Word);
  h1 = machine.ADD(&block,  h1, b, Word);
  h2 = machine.ADD(&block,  h2, c, Word);
  h3 = machine.ADD(&block,  h3, d, Word);
  Value *hlo = machine.CONCAT(&block,  h0,  h1);
  Value *hhi = machine.CONCAT(&block,  h2,  h3);
  Value *h   = machine.CONCAT(&block,  hlo, hhi);
  block.output(h);
}
catch(char const* err)
{
  printf("ERROR: %s\n", err);
  return false;
}
  return true;
}

