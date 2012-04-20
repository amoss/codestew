bool isoValues(Value *a, Value *b);
bool isoInsts(Instruction *a, Instruction *b);
template<typename T>
vector<vector<T> > partition(vector<T> source, bool (*eq)(T,T));
vector<Value*> blockDataPartition(Block *block);

void isoEntry(Block *);

