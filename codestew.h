class Instruction
{
  uint64 ref, opcode, numInputs, inputsMax, numOutputs, outputsMax;
  Value *inputs, *outputs;
};

class Value
{
  uint64 ref, typecode, numUses, usesMax;
  Instruction *def;
  Instruction *uses;
  uint64 ref;
};

class Block
{
  uint64 numInsts, instsMax, numValues, valuesMax;
  Instruction *insts;
  Values *values;

  Value *createValue();
  Instruction *createInstruction(uint64 opcode);
};
