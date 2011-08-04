import codestew

block = codestew.block()
word  = codestew.ubits(64)
in0   = block.createInput(word)
in1   = block.createInput(word)
t1    = block.createInstruction( ?xor?, [in0,in1], [word] )
t2    = block.createInstruction( ?xor?, [in0,t1],  [word] )
block.markOutput(t2)

