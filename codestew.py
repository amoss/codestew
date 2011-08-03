# Things to store (vertices):
#  Instructions
#  Values
#    Types (transparent / extensible)
#  I/O
# Things to store (edges) :
#  Inst -> Val
#  Val -> Inst

# Must support:
#   CAO-style 
#     Parsing / Code-gen
#     Transformation / mutation
#   Interpolator
#     Combinatoric generation
#   Timing probes
#     Combinatoric generation
#   Poweranal
#     Parsing / Code-gen
#     Transformation / mutation

# Output:
#   x86-64
#   ARM
#   Register allocation

# What is the right way to represent an instruction?
# How do we represent a value, and an extensible type?
# Because every instruction and value is a vertex in the graph we do not alias values.
# Although SSA is designed to make aliasing explicit a standard implementation in which
# there is a label inside an instruction in a code block is still aliasing a value. This
# means that properties such referential integrity are can be broken.

# LLVM:
#   Fixes the set of instructions and types, provides simple semantics and safety
#   Abstracts register allocation (same as Qhasm).
#   Infinite set of word values
#   Load/Store architecture (partition between values and mem-repr/addresses)
#     Explicit memory allocation 
#     Typed memory
#   SSA
#   Related work compares against high-level VMs...
#   Differences:
#     Do not fix the semantics of instructions / types / values
#     Allows representation of a series of languages
#     Input language domain specific, output language is architecture-dependent.
#     Guaranteed step-down between languages once optimisation is exhausted.
#     Sequential bblocks?
# SafeTSA:
#   Differences:
#     Our IR has referential integrity by definition...
# UNCOL:
# ANDF:


# What about creation routines that make vertices in a "pool", but they are not
# in the block until they are linked?
class block(object) :
  counter = 1

  # Do we want values without types ever?
  def createValue(self, name=None, typeinfo="word") :
    pass

  # Do we want unlinked instructions?
  def createInstruction(self, opcode) :
    pass

  def linkValInst(self, value, inst, sourceIdx) :
    pass

  def linkInstVal(self, inst, tarIdx, value) :
    pass

  # AT some point we want to create an interface like...
  def createAdd(self, op1, op2) :
    return result...
