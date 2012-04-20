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

  // Match? -> yes, no, so-far...
};

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

class CanonSet
{
public:
  vector<Canon> matches;

  vector< vector<Canon> > expand()
  {
    for(int i=0; i<matches.size(); i++)
      matches[i].possibles = partition<Instruction*>(matches[i].vals[0]->uses, isoInsts);
    return partition<Canon>(matches,eqPosShapes);
  }
};


void isoEntry(Block *block)
{
vector<vector<Value*> > bins = blockDataPartition(block);

  for(int i=0; i<bins.size();)
  {
    if(bins[i].size() < 5)
      bins.erase( bins.begin()+i );
    else
      i++;
  }

vector<CanonSet> canons;
  for(int i=0; i<bins.size(); i++)
  {
    CanonSet cs;
    for(int j=0; j<bins[i].size(); j++)
    {
      Canon c;
      c.vals.push_back( bins[i][j] );
      cs.matches.push_back(c);
    }
    canons.push_back(cs);
  }

  for(int i=0; i<canons.size();)
  {
    vector< vector<Canon> > split = canons[i].expand();
    if( split.size()>1 )
    {
      // Bit inefficient as they will be processed again but are all homogeneous
      canons.erase( canons.begin()+i );
      for(int j=0; j<split.size(); j++)
      {
        CanonSet cs;
        cs.matches = split[j];
        canons.push_back(cs);
      }
    }
    else
      i++;
  }

  for(int i=0; i<canons.size(); i++)
  {
      printf("Bin %d\n", i);
      for(int j=0; j<canons[i].matches.size(); j++)
      {
        Value *v = canons[i].matches[j].vals[0];
        printf("  %s\n", v->repr().c_str());
        //vector<vector<Instruction *> > useBins =
        //        partition<Instruction*>(v->uses, isoInsts);
        vector<vector<Instruction*> > &useBins = canons[i].matches[j].possibles;
        //for(int k=0; k<canons[i].matches[j].possibles.size(); k++)
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
