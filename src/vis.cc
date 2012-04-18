#include "vis.h"

// Set theoretic operations:
//    intersection
//    union
//    subtract

RegionX::RegionX(Block *b)
{
  block = b;
  ninsts = b->numInsts();
  nvals  = b->numValues();
  insts = new bool[ninsts];
  vals  = new bool[nvals];
  clear();
  for(int i=0; i<nvals; i++)
  {
    Value *v = block->getValue(i);
    if(v->constant!=NULL)
      vals[i] = true;
  }
}

void RegionX::clear()
{
  for(int i=0; i<ninsts; i++)
    insts[i] = false;
  for(int i=0; i<nvals; i++)
    vals[i] = false;
}

void RegionX::copyFrom(RegionX *src)
{
  ASSERT(src->block == block);
  for(int i=0; i<ninsts; i++)
    insts[i] = src->insts[i];
  for(int i=0; i<nvals; i++)
    vals[i] = src->vals[i];
}

void RegionX::markOutputs(int idx)
{
Instruction *inst = block->getInst(idx);
  for(int i=0; i<inst->outputs.size(); i++)
    vals[ inst->outputs[i]->ref ] = true;
}

void RegionX::mark(Value *v)
{
  ASSERT(v->ref < nvals);
  vals[ v->ref ] = true;
}

void RegionX::markDefinedUses(int idx)
{
Value *val = block->getValue(idx);
  //printf("Mark up to %zu uses of %d\n", val->uses.size(), idx);
  for(int i=0; i<val->uses.size(); i++)
  {
    Instruction *inst = val->uses[i];
    int defined;
    for(defined=0; defined<inst->inputs.size(); defined++)
    {
      //printf("Check %d %llu\n",defined,inst->inputs[defined]->ref);
      if( !vals[ inst->inputs[defined]->ref ] )
        break;
    }
    if( defined < inst->inputs.size() )
      continue;
    insts[ inst->ref ] = true;
  }
}

void RegionX::unionFrom(RegionX *other)
{
  ASSERT( other->block == block );
  for(int i=0; i<ninsts; i++)
    if(other->insts[i])
      insts[i] = true;
  for(int i=0; i<nvals; i++)
    if(other->vals[i])
      vals[i] = true;
}

void RegionX::expandToDepth(int n)
{
RegionX done(block);
  done.copyFrom(this);
  //printf("Done: %s\n", done.repr().c_str());

  for(int step=0; step<n; step++)
  {
    printf("Step %d\n",step);

    for(int i=0; i<done.nvals; i++)
      if( done.vals[i])
        done.markDefinedUses(i);

    //printf("Frontier: %s\n", done.repr().c_str());

    // For every instruction marked
    for(int i=0; i<done.ninsts; i++)
      if( done.insts[i] )
        done.markOutputs(i);

    //printf("Frontier: %s\n", done.repr().c_str());

  }

  unionFrom(&done);
}

void RegionX::dot(const char *filename)
{
FILE *f = fopen(filename,"w");
  if(f==NULL)
    throw "Cannot open output file";
  fprintf(f,"digraph{\n");
  fprintf(f,"input [shape=none];\noutput [shape=none];\n");
  for(int i=0; i<nvals; i++)
    if( vals[i] )
      fprintf(f,"v%d [label=\"%d : %s\"];\n", i, i, 
                block->getValue(i)->type->repr().c_str());
  for(int i=0; i<block->numInputs(); i++)
    if( vals[ block->getInput(i)->ref ] )
      fprintf(f,"input -> v%llu [color=grey];\n", block->getInput(i)->ref);
  for(int i=0; i<block->numOutputs(); i++)
    fprintf(f,"v%llu -> output [color=grey];\n", block->getOutput(i)->ref);

  for(int i=0; i<ninsts; i++)
  {
    if(!insts[i])
      continue;
    Instruction *inst = block->getInst(i);
    fprintf(f,"i%d [shape=none,label=\"%s\"];\n", i, inst->opcode->name);
    for(int j=0; j<inst->inputs.size(); j++)
      fprintf(f,"v%llu -> i%d;\n", inst->inputs[j]->ref, i);
    for(int j=0; j<inst->outputs.size(); j++)
      fprintf(f,"i%d -> v%llu;\n", i, inst->outputs[j]->ref);
  }
  fprintf(f,"}");
  fclose(f);
}

string RegionX::reprValue(uint64 idx)
{
char result[64];
  if( vals[idx] )
    sprintf(result, "%llu(%zu)", idx, block->getValue(idx)->uses.size());
  else
    sprintf(result, "%llu(-)", idx);
  return string(result);
}

string RegionX::repr()
{
char result[16384] = {0};
  for(int i=0; i<nvals; i++)
    if( vals[i] )
      sprintf(result+strlen(result),"d%u ",i);

  for(int i=0; i<ninsts; i++)
  {
    if( insts[i] )
    {
      Instruction *inst = block->getInst(i);
      if( inst->inputs.size()>0 )
      {
        sprintf(result+strlen(result), "%s", reprValue(inst->inputs[0]->ref).c_str()); 
        for(int j=1; j<inst->inputs.size(); j++)
          sprintf(result+strlen(result), ",%s", reprValue(inst->inputs[j]->ref).c_str());
      }
      else
        sprintf(result+strlen(result), "()");

      sprintf(result+strlen(result),"->i%u->",i);

      if( inst->outputs.size()>0 )
      {
        sprintf(result+strlen(result), "%s", reprValue(inst->outputs[0]->ref).c_str()); 
        for(int j=1; j<inst->outputs.size(); j++)
          sprintf(result+strlen(result), ",%s", reprValue(inst->outputs[j]->ref).c_str());
      }
      else
        sprintf(result+strlen(result), "()");
      sprintf(result+strlen(result), " ");
    }
  }
  return string(result);
}

void RegionX::invert()
{
  for(int i=0; i<ninsts; i++)
    insts[i] = !insts[i];
  for(int i=0; i<nvals; i++)
    vals[i] = !vals[i];
}




/*Region::Region(Block *b, int seed)
{
  block = b;
  insts.push_back(seed);
}

void Region::expandValues()
{
  for(int i=0; i<insts.size(); i++)
  {
    Instruction *inst = block->getInst( insts[i] );
    for(int j=0; j<inst->inputs.size(); j++)
      vals.insert( inst->inputs[j]->ref );
    for(int j=0; j<inst->outputs.size(); j++)
      vals.insert( inst->outputs[j]->ref );
  }
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
    if( instA->opcode != instB->opcode || instA->inputs.size() != instB->inputs.size())
      return false;
    for(int j=0; j<instA->inputs.size(); j++)
    {
      Value *valA = instA->inputs[j];
      Value *valB = instB->inputs[j];
      if( vals.find(valA->ref) == vals.end() && vals.find(valB->ref) == vals.end() )
        continue;
      if( valA->type != valB->type || valA->uses.size() != valB->uses.size())
        return false;
    }
  }
  return true;
}

string Region::reprInput(uint64 inp)
{
char result[64];
  sprintf(result, "%llu(%zu)", inp, block->getValue(inp)->uses.size());
  return string(result);
}

vector<string> Region::filterInputs(Instruction *inst)
{
vector<string> result;
  for(int i=0; i<inst->inputs.size(); i++)
    //if( find(vals.begin(), vals.end(), inst->inputs[i]->ref )!=vals.end() )
      result.push_back( reprInput(inst->inputs[i]->ref) );
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

string joinStr(vector<string> arr, char delim)
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
    vector<string> ins  = filterInputs( block->getInst(inum) );
    string instr  = joinStr(ins, ',');
    if( instr.length() > 0 )
      sprintf(result+strlen(result),"%s->", instr.c_str());

    sprintf(result+strlen(result),"i%llu",inum);

    vector<uint64> outs  = filterOutputs( block->getInst(inum) );
    string outstr  = joinStr(outs, ',');
    if( outstr.length() > 0 )
      sprintf(result+strlen(result),"->%s", outstr.c_str());

    if(i<insts.size()-1)
      sprintf(result+strlen(result)," ");

  }
  return string(result);
}

void dumpPartition(Block *block, vector<vector<Region> > &bins)
{
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

void partitionInsert(vector<vector<Region> > &bins, Region newR)
{
int j;
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
}*/

/*void sizeOne(Block *block)
{
vector<vector<Region> > bins;
int j;
  for(int i=0; i<block->numInsts(); i++)
  {
    Region newR = Region(block,i);
    partitionInsert(bins,newR);
  }
  dumpPartition(block,bins);
}

void sizeTwo(Block *block)
{
vector<vector<Region> > bins;
int j;
  for(int i=0; i<block->numInsts(); i++)
  {
    Region newR = Region(block,i);
    newR.expandValues();
    partitionInsert(bins,newR);
  }
  dumpPartition(block,bins);
}*/

void partition(Block *block)
{
  //sizeOne(block);
  //sizeTwo(block);
RegionX header(block);
  for(int i=0; i<block->numInputs(); i++)
    header.mark( block->getInput(i) );
  header.expandToDepth(2);

  printf("RegionX: %s\n", header.repr().c_str() );
  header.dot("crap.dot");
  header.invert();
  header.dot("crap2.dot");
}
// DFS (bounded)
// dominated regions
