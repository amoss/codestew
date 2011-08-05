import pycodestew

block = pycodestew.Block()
word  = pycodestew.Type("ubits",64)
mac   = pycodestew.SimpleMachine()
in0   = block.input(word)
print in0
in1   = block.input(word)
t1    = mac.XOR( block, (in0,in1), word )
t2    = mac.XOR( block, (in0,t1),  word )
block.markOutput(t2)

