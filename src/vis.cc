#include "vis.h"


Region::Region(Block *b, int seed)
{
  block = b;
  insts.push_back(seed);
}

// Regions use ordered sets of insts and vals, this is possible because they are
// grown incrementally with each intermediate step preserving an equality relation.
// i.e. the positions of insts and vals within these ordered sets is equivalent to a 
//      isomorphism between two Regions.
bool Region::equality(Region *other)
{
  if(other->block != block)
    return false;
  if(other->insts.size() != insts.size() || other->vals.size() != vals.size())
    return false;
  for(int i=0; i<insts.size(); i++)
  {
    Instruction *instA = block->getInst(insts[i]);
    Instruction *instB = block->getInst(other->insts[i]);
    if( instA->opcode != instB->opcode)
      return false;
    for(int j=0; j<instA->inputs.size(); j++)
    {
      Value *valA = instA->inputs[j];
      Value *valB = instB->inputs[j];
      if( valA->type != valB->type || valA->uses.size() != valB->uses.size())
        return false;
    }
  }
  return true;
}

vector<uint64> Region::filterInputs(Instruction *inst)
{
vector<uint64> result;
  for(int i=0; i<inst->inputs.size(); i++)
    //if( find(vals.begin(), vals.end(), inst->inputs[i]->ref )!=vals.end() )
      result.push_back(inst->inputs[i]->ref);
  return result;
}

vector<uint64> Region::filterOutputs(Instruction *inst)
{
vector<uint64> result;
  for(int i=0; i<inst->outputs.size(); i++)
    //if( find(vals.begin(), vals.end(), inst->outputs[i]->ref )!=vals.end() )
      result.push_back(inst->outputs[i]->ref);
  return result;
}

string joinStr(vector<uint64> arr, char delim)
{
stringstream build;
  for(int i=0; i<arr.size(); i++)
  {
    build << arr[i];
    if( i<arr.size()-1 )
      build << delim;
  }
  return build.str();
}

string Region::repr()
{
char result[2048] = {0};
  for(int i=0; i<insts.size(); i++)
  {
    uint64 inum = insts[i];
    vector<uint64> ins  = filterInputs( block->getInst(inum) );
    string instr  = joinStr(ins, ',');
    if( instr.length() > 0 )
      sprintf(result+strlen(result),"%s->", instr.c_str());

    sprintf(result+strlen(result),"i%u",inum);

    vector<uint64> outs  = filterOutputs( block->getInst(inum) );
    string outstr  = joinStr(outs, ',');
    if( outstr.length() > 0 )
      sprintf(result+strlen(result),"->%s", outstr.c_str());

    if(i<insts.size()-1)
      sprintf(result+strlen(result)," ");

  }
  return string(result);
}

void partition(Block *block)
{
vector<vector<Region> > bins;
int j;
  for(int i=0; i<block->numInsts(); i++)
  {
    Region newR = Region(block,i);
    Instruction *inst = block->getInst(i);
    for(j=0; j<inst->inputs.size(); j++)
      newR.vals.push_back(inst->inputs[j]->ref);
    for(j=0; j<inst->outputs.size(); j++)
      newR.vals.push_back(inst->outputs[j]->ref);
    for(j=0; j<bins.size(); j++)
      if( newR.equality( &bins[j][0] ) )
      {
        bins[j].push_back(newR);
        break;
      }
    if( j==bins.size() )
    {
      vector<Region> newBin;
      newBin.push_back(newR);
      bins.push_back(newBin);
    }
  }

  for(int i=0; i<bins.size(); i++)
  {
    printf("Bin %d (%s) : ", i, block->getInst(bins[i][0].insts[0])->opcode->name);
    for(int j=0; j<bins[i].size(); j++)
    {
      printf("%s ", bins[i][j].repr().c_str());
    }
    printf("\n");
  }
}


/*void partition(Block *block)
{
map<char const *,set<int> > bins;    // opcode -> instruction nums
  for(int i=0; i<block->numInsts(); i++)
  {
    Instruction *inst = block->getInst(i);
    if( bins.find(inst->opcode->name)==bins.end() )
      bins[ inst->opcode->name ] = set<int>();
    bins[ inst->opcode->name ].insert(i);
  }

map<char const *,set<int> >::iterator it;
  for(it = bins.begin(); it!=bins.end(); ++it)
  {
    printf("Bin (%s) : ", it->first);
    set<int>::iterator it2;
    for(it2 = it->second.begin(); it2!=it->second.end(); ++it2)
      printf("%u ", *it2);
    printf("\n");
  }

    
}*/
