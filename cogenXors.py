import pycodestew

block = pycodestew.Block()
word  = pycodestew.Ubits(64)
mac   = pycodestew.SimpleMachine()
in0   = block.createInput(word)
in1   = block.createInput(word)
t1    = mac.XOR( [in0,in1], [word] )
t2    = mac.XOR( [in0,t1],  [word] )
block.markOutput(t2)

