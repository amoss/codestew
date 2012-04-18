#include "codestew.h"
#include <sstream>

using namespace std;
/* The purpose of a Region is to allow partitioning of sets of digraph according to
   an equivalence relation. There are three broad approaches:
    1. A boolean comparison between two Region objects.
    2. An ordering predicate over Region objects.
    3. Projection from a Region object onto a canonical representative in a more 
       structured set (such as the integers).
   In terms of usability and performance option (3) is a clear winner, however hashing
   of regions is tricky, and hashes that are guaranteed to produce unique values over
   particular subsets of all regions are even trickier.
   Option (2) could yield a performance improvement over option (1) as the partitioning
   of a set of Regions will be O(n^2) using a boolean comparison, but could be improved
   to O(n*log n) using an ordering. Currently the potential improvement is not worth
   the effort of working out how to map the Region into a Godel numbering that respects
   the structural properties of the graph.
*/

class RegionX
{
  Block *block;
  bool *insts, *vals;
  int ninsts, nvals;
public:
  RegionX(Block *);
  string repr();
  string reprValue(uint64);
  void dot(const char *);

  void mark(Value *);
  void copyFrom(RegionX *);
  void unionFrom(RegionX *x);
  //void markOutputs(Instruction *inst);
  void markOutputs(int );
  void markDefinedUses(int);
  void clear();
  void expandToDepth(int);

  void invert();
};

#define ASSERT(COND)


/*class Region
{
public:
  Block *block;
  vector<int> insts;
  set<int> vals;
  Region(Block *b, int seed);

  void expandValues();
  bool equality(Region *other);

  string reprInput(uint64);

  vector<string> filterInputs(Instruction *);
  vector<uint64> filterOutputs(Instruction *);

  

  string repr();
};*/

void partition(Block *);

