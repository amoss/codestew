#include "vis.h"
#include <stdio.h>

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

// Arbitrary ordering over blocks within a partition to make distributions comparable.
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

// TODO: Implement
//       Should this be related to the eq buried insid incProductions?
//       If all the eqs/orders pair up are we using families of relations over the graph?
bool orderVals(Value *a, Value *b)
{
  if( a->uses.size() < b->uses.size() )
    return true;
  return false;
}

// Higher-order version of isoVals. Compares blocks of vals within a partition.
bool eqValBlock(vector<Value*> a, vector<Value*> b)
{
  if(a.size()!=b.size())  
    return false;
  for(int i=0; i<a.size(); i++)
    if(!isoValues(a[i],b[i]))
      return false;
  return true;
}

// Isomorphism
//   [IsoRegion]
//     [Section]

/* An Isomorphism is a mapping between sub-graphs of a Block that preserves the 
   types of vertices and the edge structure within the induced sub-graph. This
   implies that each vertex has a canonical numbering so that the vertex in each
   sub-graph with the same number is equivalent wrt to its position within every 
   trace within the sub-graph. The isomorphism between every pair of sub-graphs
   is equivalent upto, but not including, the unique name of every vertex.
       An IsoRegion holds information about the structure of each isomorphic 
   sub-graph that is built up incrementally during the search process and reaches
   a stable state in which the canonical numbering represented by the position of
   the vertices within its container. The set of IsoRegions in an Isomorphism is 
   built up in lock-step. As well as the canonical numbering implied by the storage
   representation there is an implication that part of the isomorphism 
   representation is stored across the set of IsoRegions.
       A Section is a sub-graph within a Block that contains a single Value and the
   set of Instructions that use that Value. The set of Instructions is partitions
   according to distinguishability among the elements of the set. Elements that 
   can be uniquely distinguished (e.g. by opcode) are referred to as definite 
   because the isomorphic region can only be extended to include the Instruction
   iff the Instruction matches across all IsoRegions. Elements that are not
   locally unique (e.g. multiple Instructions with matching opcodes and operand
   types) may be within the isomorphic regions but only if some unknown permutation
   within each IsoRegion/Section produces a canonical numbers that is consistent
   across the entire Isomorphism.
*/

class Section
{
public:
Value *head;
vector<Instruction *> definite;
vector<vector<Instruction*> > possibles;
  Section(Value *v)
  {
    head = v;
    possibles = partition<Instruction*>(head->uses, isoInsts);
    sort(possibles.begin(), possibles.end(), orderPos);
  }

  vector<Value*> confirm()
  {
    vector<Value*> orderedVals;
    for(int i=0; i<possibles.size(); )
      if(possibles[i].size()==1)
      {
        definite.push_back(possibles[i][0]);
        // When we confirm an instruction, it's outputs are strictly ordered so can
        // be confirmed as well. As the block is SSA each confirmed Value only has
        // an incoming edge from the confirmed instruction.
        for(int j=0; j<possibles[i][0]->outputs.size(); j++)
          orderedVals.push_back(possibles[i][0]->outputs[j]);
        possibles.erase( possibles.begin()+i );
      }
      else
        i++;
    return orderedVals;
  }

  int locate(Instruction *inst)
  {
    for(int i=0; i<possibles.size(); i++)
    {
      vector<Instruction*> &check = possibles[i];
      if( find(check.begin(), check.end(), inst) != check.end() )
        return i;
    }
    return -1;
  }
};

class IsoRegion
{
public:
  vector<Section> sections;

  IsoRegion()
  {
  }

  IsoRegion(Section init)
  {
    sections.push_back(init);
  }

  int locate(Instruction *inst)
  {
    for(int i=0; i<sections.size(); i++)
    {
      vector<Instruction*> &check = sections[i].definite;
      if( find(check.begin(), check.end(), inst) != check.end() )
        return i;
      for(int j=0; j<sections[i].possibles.size(); j++)
      {
        vector<Instruction*> &check = sections[i].possibles[j];
        if( find(check.begin(), check.end(), inst)!=check.end() )
          return i;
      }
    }
    return -1;
  }

  void confirm()
  {
    vector<Value*> newHeads;
    for(int i=0; i<sections.size(); i++)
    {
      vector<Value*> partial = sections[i].confirm();
      newHeads.insert( newHeads.end(), partial.begin(), partial.end() );
    }
    for(int i=0; i<newHeads.size(); i++)
      sections.push_back( Section(newHeads[i]) );
  }

};

/* Equality between Sections is defined over both shape and content. The shape 
   requirements are that the definites list and the possibles distribution must be the
   same size, and the latter must have blocks within the partition of matching sizes.
   Once the shape is validated instructions are compared between blocks of the partitions
   to ensure they match.
*/
bool eqSections( Section s1, Section s2 )
{
  if( s1.possibles.size() != s2.possibles.size() ||
      s1.definite.size() != s2.definite.size() )
  {
    //printf("Section sizes %d %d %d %d\n",
    //  s1.possibles.size(), s2.possibles.size(),
    //  s1.definite.size(), s2.definite.size() );
    return false;
  }
  for(int i=0; i<s1.definite.size(); i++)
    if( !isoInsts(s1.definite[i], s2.definite[i]) )
    {
      //printf("Section def differs\n");
      return false;
    }
  for(int i=0; i<s1.possibles.size(); i++)
  {
    if(s1.possibles[i].size() != s2.possibles[i].size())
      return false;
    if(!isoInsts(s1.possibles[i][0], s2.possibles[i][0]) )
      return false;
  }
  return true;
}

/* Two regions are equal if they have the same number of sections, and the above
   equality holds pointwise across the sections in the list of regions.
*/
bool eqRegions( IsoRegion i1, IsoRegion i2)
{
  if( i1.sections.size() != i2.sections.size() )
  {
    //printf("Section sizes %d %d\n", i1.sections.size(), i2.sections.size() );
    return false;
  }
  for(int s=0; s<i1.sections.size(); s++)
  {
    if( !eqSections(i1.sections[s], i2.sections[s]) )
    {
      //printf("Section ineq %d\n",s);
      return false;
    }
  }
  return true;
}

// Contains a set of Canon object - each represents a region in the same block and
// a canonisation of the vertices to allow the isomorphic mapping between them.
class Isomorphism
{
public:
  vector<IsoRegion> regions;
  // Temp default
  Isomorphism()
  {
  }

  // Replacement constructor for initByValues
  Isomorphism(vector<Value*> seeds)
  {
    for(int i=0; i<seeds.size(); i++)
      regions.push_back(IsoRegion(Section(seeds[i])));
  }

  // Replacement constructor for expand
  Isomorphism(vector<IsoRegion> regions)
  {
    this->regions = regions;
  }

  /* Find which IsoRegion the Instruction is within
  */
  int locate(Instruction *inst)
  {
    for(int i=0; i<regions.size(); i++)
    {
      for(int j=0; j<regions[i].sections.size(); j++)
      {
        vector<Instruction*> &check = regions[i].sections[j].definite;
        if( find(check.begin(), check.end(), inst)!=check.end() )
          return i;
        for(int k=0; k<regions[i].sections[j].possibles.size(); k++)
        {
          vector<Instruction*> &check = regions[i].sections[j].possibles[k];
          if( find(check.begin(), check.end(), inst)!=check.end() )
            return i;
        }
      }
    }
    return -1;
  }

  static vector<Isomorphism> initialSplit(Block *block)
  {
  vector<Value*> values;
    for(int i=0; i<block->numValues(); i++)
      values.push_back(block->getValue(i));
    vector<vector<Value*> > seeds = partition<Value*>(values, isoValues);

  vector<Isomorphism> result;
    for(int i=0; i<seeds.size(); i++)
      result.push_back(Isomorphism(seeds[i]));
    return result;
  }

  /* Using the equality definitions on IsoRegions and Sections check that every
     IsoRegion within an Isomorphism is indistinguishable. Where some distinction
     exists partition the IsoRegions into equivalence classes, and re-wrap each
     block of the partition into a new Isomorphism.
         Where update operations to extend the IsoRegions across the block cause
     differences to appear this operation splits the Isomorphism into separate 
     valid Isomorphisms.
  */
  vector<Isomorphism> distinguish()
  {
    vector<vector<IsoRegion> > split = partition<IsoRegion>(regions, eqRegions);
    vector<Isomorphism> result;
    for(int i=0; i<split.size(); i++)
      result.push_back(Isomorphism(split[i]));
    return result;
  }

  /* Scan the IsoRegion and output the <section,block> indices for the blocks within
     the possibles partition. If the region/section/blocks storage is viewed as a 3D
     array then this function computes the indices of a section/blocks slice for a 
     fixed region.
  */
  vector< pair<int,int> > possiblesBlocks(int region)
  {
    vector< pair<int,int> > result;   // Indices for the slice of this image
    IsoRegion &r = regions[region];
    for(int i=0; i<r.sections.size(); i++)
    {
      for(int j=0; j<r.sections[i].possibles.size(); j++)
        result.push_back( pair<int,int>(i,j) );
    }
    return result;
  }

  /* Create a slice of <section,block> from every IsoRegion. The slice does not
     preserve topology, collapse the blocks into a flat list.
  */
  vector< vector<Instruction*> > slicePosBlocks(int section, int block)
  {
    vector< vector<Instruction*> > productions;
    for(int i=0; i<regions.size(); i++)
      productions.push_back(regions[i].sections[section].possibles[block]);
    return productions;
  }

  /* TODO: Really need to fix the horrific hack by deciding what we will do with
           multi-return instructions.
  */
  vector< vector<Value*> > mapBlocksToProds(vector< vector<Instruction*> > producers)
  {
    vector< vector<Value*> > result;
    for(int i=0; i<producers.size(); i++)
    {
      vector<Value*> blockVals;
      for(int j=0; j<producers[i].size(); j++)
      {
        if(producers[i][j]->outputs.size()!=1)
        {
          printf("Multi-output Instructions not classified yet!\n");
          exit(-1);
        }
        blockVals.push_back(producers[i][j]->outputs[0]);
      }
      result.push_back(blockVals);
    }
    return result;
  }

  // two-stepper
  // 1. ordering block be valIso
  // 2. splitting isomorphism to remove non-matching
  vector<Isomorphism> incProductions()
  {
    // Assume that all IsoRegions are currently matching so pick an arbitrary region
    // to scan for the possibles blocks.
    vector< pair<int,int> > blImages = possiblesBlocks(0);

    // TODO: The tail of this function needs to look over elements in blImages
    int sIdx = blImages[0].first;
    int pIdx = blImages[0].second;

    vector< vector<Instruction*> > producers = slicePosBlocks(sIdx,pIdx);
    vector< vector<Value*> > productions     = mapBlocksToProds(producers);


    // Sort each block of Values projected from the Instruction blocks, then partition
    // by equality between blocks.
    vector< vector< vector<Value* > > > valSplit =
                                    partition< vector<Value*> >(productions, eqValBlock);
    for(int i=0; i<valSplit.size(); i++)
      for(int j=0; j<valSplit[i].size(); j++)
        sort(valSplit[i][j].begin(), valSplit[i][j].end(), orderVals);

    // Ideas for refactoring:
    //   Confirming each of the possibles instructions
    //   Slicing?

    // TODO: We are restructuring every iso split from this one. We only want to split
    //       the one that we are keeping, or if we keep the others do each iso relative
    //       to its first element rather than zero in the second index below.
    for(int i=0; i<valSplit.size(); i++)  // Foreach isomorphism
    {
      for(int j=0; j<valSplit[i][0].size(); ) // Foreach pos in vals proj from possibles
      {
        if( j+1 == valSplit[i][0].size() || 
            !isoValues(valSplit[i][0][j],valSplit[i][0][j+1]) ) 
        {
          // Confirm an instruction within every region. For every region find the 
          // section and possibles position of the newly confirmed instruction.
          for(int k=0; k<valSplit[i].size(); k++)
          {
            Instruction *def = valSplit[i][k][j]->def;
            int srcRegion  = locate(def);
            int srcSection = regions[srcRegion].locate(def);
            printf("Confirm %d,%d,%d<-%d,%d : %s\n", i,j,k, 
                  srcRegion, srcSection, def->repr().c_str());
            // Find out where it is in the possibles
            int srcPosBlock = regions[srcRegion].sections[srcSection].locate(def);
            vector<Instruction *> &srcBlock = 
                regions[srcRegion].sections[srcSection].possibles[srcPosBlock];
            srcBlock.erase( find( srcBlock.begin(), srcBlock.end(), def ));
            // erase it and push_back to definites.
            regions[srcRegion].sections[srcSection].definite.push_back(def);
            // TODO:Haven't propagated to next val, needs to consider boundaries
          }


          j++;
        }
        else
        {
          // Skip past the isovalues 
          while( j<valSplit[i][0].size() && 
                 isoValues(valSplit[i][0][j],valSplit[i][0][j+1]) )
            j++;
        }
        
      }
    }


    vector<Isomorphism> results;
    for(int i=0; i<valSplit.size(); i++)
    {
      vector<IsoRegion> matches;
      for(int j=0; j<valSplit[i].size(); j++)
      {
        Instruction *creator = valSplit[i][j][0]->def;
        int idx = locate(creator);
        matches.push_back(regions[idx]);
      }
      results.push_back(Isomorphism(matches));
    }

    return results;

    // Dump the two-step results
    /*
    for(int i=0; i<valSplit.size(); i++)
    {
      printf("Block %d: (%d)\n", i, valSplit[i].size());
      for(int j=0; j<valSplit[i].size(); j++)
      {
        printf("  ");
        for(int k=0; k<valSplit[i][j].size(); k++)
          printf("%s,%zu ",valSplit[i][j][k]->repr().c_str(), valSplit[i][j][k]->uses.size());
        printf("\n");
      }
    }
    */
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
    for(int i=0; i<regions.size(); i++)
      regions[i].confirm();
  }

  vector<RegionX> eqClassRegions(Block *block)
  {
    char colName[64];
    vector<RegionX> result;
    for(int i=0; i<regions[0].sections.size(); i++)
    {
      sprintf(colName,"style=filled,fillcolor=\"/spectral11/%d\"",i+1); 
      RegionX r = RegionX(block,colName);
      for(int j=0; j<regions.size(); j++)
        r.mark(regions[j].sections[i].head);
      result.push_back(r);
    }
    int defCounter=0;
    for(int i=0; i<regions[0].sections.size(); i++)
    {
      for(int j=0; j<regions[0].sections[i].definite.size(); j++)
      {
        sprintf(colName,"style=filled,fillcolor=\"/purples6/%d\"",defCounter+2); 
        RegionX r = RegionX(block,colName);
        for(int k=0; k<regions.size(); k++)
          r.mark(regions[k].sections[i].definite[j]);
        result.push_back(r);
        defCounter++;
      }
    }
    int posCounter=0;
    for(int i=0; i<regions[0].sections.size(); i++)
    {
      for(int j=0; j<regions[0].sections[i].possibles.size(); j++)
      {
        sprintf(colName,"style=filled,fillcolor=\"/ylorrd8/%d\"",posCounter+1); 
        RegionX r = RegionX(block,colName);
        for(int k=0; k<regions.size(); k++)
          for(int l=0; l<regions[0].sections[i].possibles[j].size(); l++)
            r.mark(regions[k].sections[i].possibles[j][l]);
        result.push_back(r);
        posCounter++;
      }
    }
    return result;
  }

  void dump()
  {
    for(int j=0; j<regions.size(); j++)
    {
      printf("Region %d\n", j);
      IsoRegion &cur = regions[j];
      for(int k=0; k<cur.sections.size(); k++)
      {
        printf("  Section %d\n", k);
        Value *v = cur.sections[k].head;
        printf("  %s\n", v->repr().c_str());
        for(int l=0; l<cur.sections[k].definite.size(); l++)
          printf("  %d,%d: %s\n", k, l, cur.sections[k].definite[l]->repr().c_str());
        vector<vector<Instruction*> > &useBins = cur.sections[k].possibles;
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
};

// There are two sensible measures of size for an Isomorphism:
//   the number of regions it maps onto (we term commonality)
//   the number of vertices inside the region (we term population)
// To avoid filtering by one of the sizes pass it as -1
void threshold( vector<Isomorphism> &isos, int minCommon, int minPop)
{
  for(int i=0; i<isos.size();)
  {
    if(int(isos[i].regions.size()) < minCommon || 
       int(isos[i].regions[0].sections.size()) < minPop )
      isos.erase( isos.begin()+i );
    else
      i++;
  }
}

void isoEntry(Block *block)
{
vector<Isomorphism> isos = Isomorphism::initialSplit(block);

  printf("%zu blocks\n", isos.size());
  // Ignore uncommon mappings to make dev/debug easier
  threshold(isos, 5, -1);    
  printf("%zu blocks\n", isos.size());

// Splitting step -> pushed into search process eventually
  vector<Isomorphism> newSplits;
  for(int i=0; i<isos.size(); i++)
  {
    vector<Isomorphism> split = isos[i].distinguish();
    newSplits.insert( newSplits.end(), split.begin(), split.end() );
  }
  isos = newSplits;

  printf("%zu blocks\n", isos.size());

  // Push singleton blocks from possibles to definites...
  for(int i=0; i<isos.size(); i++)
    isos[i].confirm();  
  threshold(isos, 5, -1);    
  isos[4].dump(); 

  // Compare instructions in a partition block by comparing their
  // produced values to see if they look isomorphic.
  // ...This should split the adds...
  isos = isos[4].incProductions();
  for(int i=0; i<isos.size(); i++)
  {
    printf("ISO %d\n",i);
    isos[i].dump(); 
  }

  // Compare boundaries to possibles instructions according to a simple
  // classification [ Section(n) within iso (diff region) | Section(n) (same)
  // | Outside all IsoRegions ]


  // Hmmmmmmmmmmmmmmmmmmmmmm
  // Algorithm:
  //   ..?..
  //   Floodfill IsoRegions in lockstep
  //     Found a difference? either split the Isomorphism or pause in this direction
  //   ..?..
  // What happens if all of the pathways from the known part of the Isomorphism
  // go into unknown, non-overlapping, parts of the graph?

  //for(int i=0; i<isos.size(); i++)
  //{
  //  printf("Iso %d\n", i);
  //  isos[i].dump(); 
  //}

  FILE *f = fopen("crap2.dot","wt");
  RegionX remainder(block);
  for(int i=0; i<block->numInputs(); i++)
    remainder.mark( block->getInput(i) );
  remainder.markConstants();
  remainder.expandToDepth(64);
  fprintf(f,"digraph {\n");
  vector<RegionX> isoPop = isos[0].eqClassRegions(block);  // TODO: Keep changing!!
  for(int i=0; i<isoPop.size(); i++)
  {
    char colour[32];
    sprintf(colour,"pink");
    isoPop[i].intersectFrom(&remainder);
    isoPop[i].dotColourSet(f,colour);
    remainder.subtract(&isoPop[i]);
  }
  char defColour[] = "white";
  remainder.dotColourSet(f,defColour);
  fprintf(f,"}\n");
  fclose(f);
}
