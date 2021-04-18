#ifndef CS265_LSM_TREE // This is a header guard. It prevents the header from being included more than once.
#define CS265_LSM_TREE
#include "typedefs.h"
#include "buffer.h"
#include "bloom.h"
#include "functions.h"

typedef struct lsmtree {
    // define the components of the hash table here (e.g. the array, bookkeeping for number of elements, etc)
    buffer *bfr;
    int max_depth;
    int fanout;
    double error;

    level **levels;
} lsmtree;

int allocate(lsmtree** lsmt, int bfr_size, int depth, int fanout, double error);
int put(lsmtree* lsmt, keyType key, valType value);
int get(lsmtree* lsmt, keyType key, valType *value);
int erase(lsmtree* lsmt, keyType key);
int flush(lsmtree* lsmt, int index);
int deallocate(lsmtree* lsmt);
int print_statistics(lsmtree* lsmt);
int getRange(lsmtree* lsmt, keyType keyStart, keyType keyEnd, valType *value, data_chunk rangeRes[], int range);
int checkIfExist(data_chunk rangeRes[], keyType key, int size);
int getRangeWithCompaction(lsmtree *lsmt, keyType startKey, keyType endKey, valType *value, data_chunk rangeRes[], int range, float time[]);

#endif
