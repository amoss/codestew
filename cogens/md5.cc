
#include<stdio.h>
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
  Type *Chunk = new Type(Type::UBITS, 512);
  Type *Hash  = new Type(Type::UBITS, 128);

  Value *in = block.input(Chunk);

  Value *iv0 = block.constant(Word, 0x67452301);
  Value *iv1 = block.constant(Word, 0xefcdab89);
  Value *iv2 = block.constant(Word, 0x98badcfe);
  Value *iv3 = block.constant(Word, 0x10325476);


  Value *ktable[64];
  for(int i=0; i<64; i++)
    ktable[i] = block.constant(Word, (unsigned int)floor(abs(sin(i+1.0)) * pow(2,32)));

  Value *h0 = machine.COPY(iv0);
  Value *h1 = machine.COPY(iv1);
  Value *h2 = machine.COPY(iv2);
  Value *h3 = machine.COPY(iv3);
  Value *a = machine.COPY(h0);
  Value *b = machine.COPY(h1);
  Value *c = machine.COPY(h2);
  Value *d = machine.COPY(h3);

  for(int i=0; i<16; i++)
  {
    Value *t1 =    machine.LAND(b,    c            );
    Value *t2 =    machine.LNOT(b                  );
    Value *t3 =    machine.LAND(t2,   d            );
    Value *f  =     machine.LOR(t1,   t3           );
    Value *t4 =     machine.ADD(a,    f,    Word   );
    Value *t5 = machine.EXTRACT(in,   i*32, i*32+31);
    Value *t6 =     machine.ADD(k[i], t5,   Word   );
    Value *t7 =     machine.ADD(t6,   t4,   Word   );
    Value *t8 =   machine.UPROT(t7,   r[i]         );
    Value *t9 =     machine.ADD(b,    t8           );   // b'
    Value *t10=    machine.COPY(d,    Word         );   // temp
                   machine.COPY(c,    d);
                   machine.COPY(b,    c);
                   machine.COPY(t9,   b);
                   machine.COPY(t10,  a);
  }

  for(int i=16; i<32; i++)
  {
    Value *t1 =    machine.LAND(d,    b            );
    Value *t2 =    machine.LNOT(d                  );
    Value *t3 =    machine.LAND(t2,   c            );
    Value *f  =     machine.LOR(t1,   t3           );
    Value *t4 =     machine.ADD(a,    f,    Word   );
    int base = (((i*5)+1)%16) * 32;
    Value *t5 = machine.EXTRACT(in,   base, base+31);
    Value *t6 =     machine.ADD(k[i], t5,   Word   );
    Value *t7 =     machine.ADD(t6,   t4,   Word   );
    Value *t8 =   machine.UPROT(t7,   r[i]         );
    Value *t9 =     machine.ADD(b,    t8           );   // b'
    Value *t10=    machine.COPY(d,    Word         );   // temp
                   machine.COPY(c,    d);
                   machine.COPY(b,    c);
                   machine.COPY(t9,   b);
                   machine.COPY(t10,  a);
  }

  for(int i=32; i<48; i++)
  {
    Value *t1 =    machine.LXOR(b,    c            );
    Value *f  =    machine.LXOR(t1,   d            );
    Value *t2 =     machine.ADD(a,    f,    Word   );
    int base = (((i*3)+5)%16) * 32;
    Value *t3 = machine.EXTRACT(in,   base, base+31);
    Value *t4 =     machine.ADD(k[i], t3,   Word   );
    Value *t5 =     machine.ADD(t2,   t4,   Word   );
    Value *t6 =   machine.UPROT(t5,   r[i]         );
    Value *t7 =     machine.ADD(b,    t6           );   // b'
    Value *t8 =    machine.COPY(d,    Word         );   // temp
                   machine.COPY(c,    d);
                   machine.COPY(b,    c);
                   machine.COPY(t7,   b);
                   machine.COPY(t8,   a);
  }

  for(int i=48; i<64; i++)
  {
    Value *t1 =    machine.LNOT(d                  );
    Value *t2 =    machine.LOR(t1,    b            );
    Value *f  =    machine.LXOR(c,    t2           );
    Value *t3 =     machine.ADD(a,    f,    Word   );
    int base = (((i*7)+1)%16) * 32;
    Value *t4 = machine.EXTRACT(in,   base, base+31);
    Value *t5 =     machine.ADD(k[i], t4,   Word   );
    Value *t6 =     machine.ADD(t3,   t5,   Word   );
    Value *t7 =   machine.UPROT(t6,   r[i]         );
    Value *t8 =     machine.ADD(b,    t7           );   // b'
    Value *t9 =    machine.COPY(d,    Word         );   // temp
                   machine.COPY(c,    d);
                   machine.COPY(b,    c);
                   machine.COPY(t8,   b);
                   machine.COPY(t9,   a);
  }

  machine.ADD( h0, a, h0);
  machine.ADD( h1, b, h1);
  machine.ADD( h2, c, h2);
  machine.ADD( h3, d, h3);
  Value *hlo = machine.CONCAT( h0,  h1,  Pair);
  Value *hhi = machine.CONCAT( h2,  h3,  Pair);
  Value *h   = machine.CONCAT( hlo, hhi, Hash);
  Value *res = block.output(h);
}
catch(char const* err)
{
  printf("ERROR: %s\n", err);
  return false;
}
  return true;
}

