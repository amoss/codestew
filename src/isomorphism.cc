#include "vis.h"

using namespace std;

bool isoValues(Value *a, Value *b)
{
  if(a->type != b->type)
    return false;
  if(a->uses.size() != b->uses.size())
    return false;
  return true;
}

bool isoInsts(Instruction *a, Instruction *b)
{
  if(a->opcode != b->opcode)
    return false;
  if(a->inputs.size() != b->inputs.size())
    return false;
  if(a->outputs.size() != b->outputs.size())
    return false;
  for(int i=0; i<a->inputs.size(); i++)
    if(a->inputs[i]->type != b->inputs[i]->type)
      return false;
  for(int i=0; i<a->outputs.size(); i++)
    if(a->outputs[i]->type != b->outputs[i]->type)
      return false;
  return true;
}

// Given a collection of T in source split them into a partition of equivalence
// classes according to the relation eq.
template<typename T>
vector<vector<T> > partition(vector<T> source, bool (*eq)(T,T))
{
vector<vector<T> > result;
  for(int i=0; i<source.size(); i++)
  {
    // See if there is a bin holding the equivalence class for this element
    int j=0;
    for(; j<result.size(); j++)
      if(eq(source[i],result[j][0]))
      {
        result[j].push_back(source[i]);
        break;
      }
    // If there was no bin then create a new one with this element in it
    if(j==result.size())
    {
      vector<T> newBin;
      newBin.push_back(source[i]);
      result.push_back(newBin);
    }
  }
  return result;
}

vector<vector<Value*> > blockDataPartition(Block *block)
{ 
vector<Value*> values;
  for(int i=0; i<block->numValues(); i++)
    values.push_back(block->getValue(i));
  return partition<Value*>(values, isoValues);
}


class Canon
{
public:
  vector<Value*> vals;
  vector<Instruction*> definite;
  vector< vector<Instruction*> > possibles;

  void confirm()
  {
    for(int i=0; i<possibles.size(); )
      if(possibles[i].size()==1)
      {
        definite.push_back(possibles[i][0]);
        possibles.erase( possibles.begin()+i );
      }
      else
        i++;
  }
  // Match? -> yes, no, so-far...
};

bool orderPos(vector<Instruction*> a, vector<Instruction*> b)
{
  if( a[0]->opcode < b[0]->opcode )
    return true;
  if( a[0]->opcode > b[0]->opcode )
    return false;
  if( a.size() < b.size() )
    return true;
  return false;
}

// Compare by shape
// The second loop to check element sizes is currently invalid as the blocks within the
// partition stored in possibles are currently unsorted. They need to be sorted into
// some arbitrary, but canonical order.
bool eqPosShapes( Canon c1, Canon c2 )
{
  if( c1.possibles.size() != c2.possibles.size() )
    return false;
  for(int i=0; i<c1.possibles.size(); i++)
  {
    if(c1.possibles[i].size() != c2.possibles[i].size())
      return false;
  }
  return true;
}

// Contains a set of Canon object - each represents a region in the same block and
// a canonisation of the vertices to allow the isomorphic mapping between them.
class Isomorphism
{
public:
  vector<Canon> matches;

  // Wrap each Value* seed as a definite data vertex inside a Canon
  void initByValues(vector<Value*> eqClass)
  {
    for(int i=0; i<eqClass.size(); i++)
    {
      Canon c;
      c.vals.push_back( eqClass[i] );
      matches.push_back(c);
    }
  }

  // We are in a state where the Isomorphism was computed to match Values locally,
  // now we extend a step by considering the set of using Instructions and partitioning
  // according to a local equality relation on them. We not split on the Instructions
  // directly as there is an unknown permutation over their ordering. Instead we split
  // on their distribution according to the equality relation as a conservative
  // approximation of isomorphism - i.e. if the distribution differs then the two 
  // regions are definitely not isomorphic after expansion. If the distributions match
  // then they may be isomorphic...
  vector<Isomorphism> expand()
  {
    for(int i=0; i<matches.size(); i++)
    {
      matches[i].possibles = partition<Instruction*>(matches[i].vals[0]->uses, isoInsts);
      sort(matches[i].possibles.begin(), matches[i].possibles.end(), orderPos);
    }
    vector< vector<Canon> > split = partition<Canon>(matches,eqPosShapes);
    vector<Isomorphism> result;
    for(int i=0; i<split.size(); i++)
    {
      Isomorphism iso;
      iso.matches = split[i];
      result.push_back(iso);
    }
    return result; 
  }

  // Find singeleton blocks of possible instructions - as these are singleton they map
  // to exactly one instruction in each Canon within the Isomorphism. Move them into
  // the definite array so that they all have matching indices (the canonical map to
  // the isomorphism). Assumes the Isomorphism is currently valid: each Canon has 
  // identical structure upto and including the layout of blocks inside the possibles
  // partition. This implies that calling confirm() on each Canon will produce an 
  // equivalent transformation.
  void confirm()
  {
    for(int i=0; i<matches.size(); i++)
      matches[i].confirm();
    
  }
};

// Use the local equality function over Values in the supplied block to split
// them into equality classes, each class represents a set of Values that look locally
// similar (type and out-degree) wrapped in Canon objects.
vector<Isomorphism> seedByValues(Block *block)
{
vector<Isomorphism> result;
  // TODO: Maybe pull blockDataPartition inside here?
  vector<vector<Value*> > seeds = blockDataPartition(block);
  for(int i=0; i<seeds.size(); i++)
  {
    Isomorphism valEqClass;
    valEqClass.initByValues( seeds[i] );
    result.push_back(valEqClass);
  }
  return result;
}

// There are two sensible measures of size for an Isomorphism:
//   the number of regions it maps onto (we term commonality)
//   the number of vertices inside the region (we term population)
// To avoid filtering by one of the sizes pass it as -1
void threshold( vector<Isomorphism> &isos, int minCommon, int minPop)
{
  for(int i=0; i<isos.size();)
  {
    if(int(isos[i].matches.size()) < minCommon || 
       int(isos[i].matches[0].vals.size()) < minPop )
      isos.erase( isos.begin()+i );
    else
      i++;
  }
}

void isoEntry(Block *block)
{
vector<Isomorphism> isos = seedByValues(block);

  printf("%zu blocks\n", isos.size());
  // Ignore uncommon mappings to make dev/debug easier
  threshold(isos, 5, -1);
  printf("%zu blocks\n", isos.size());

// Splitting step -> pushed into search process eventually
  vector<Isomorphism> newSplits;
  for(int i=0; i<isos.size(); i++)
  {
    vector<Isomorphism> split = isos[i].expand();
    newSplits.insert( newSplits.end(), split.begin(), split.end() );
  }
  isos = newSplits;

  printf("%zu blocks\n", isos.size());

  // Push singleton blocks from possibles to definites...
  for(int i=0; i<isos.size(); i++)
    isos[i].confirm();

  // Can also use ordering of outputs in new definites to push values as well...

  // Need to convert a Canon into a Region...

  for(int i=0; i<isos.size(); i++)
  {
      printf("Bin %d\n", i);
      for(int j=0; j<isos[i].matches.size(); j++)
      {
        Value *v = isos[i].matches[j].vals[0];
        printf("  %s\n", v->repr().c_str());
        vector<vector<Instruction*> > &useBins = isos[i].matches[j].possibles;
        for(int k=0; k<useBins.size(); k++)
        {
          printf("    ");
          for(int l=0; l<useBins[k].size(); l++)
            printf("%s ",useBins[k][l]->repr().c_str());
          printf("\n");
        }
      }
  }
}
