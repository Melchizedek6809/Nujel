#ifndef NUJEL_LIB_ALLOC_ALLOCATOR
#define NUJEL_LIB_ALLOC_ALLOCATOR
#include "../nujel.h"


#define defineAllocator(T, typeMax) \
extern T T##List[typeMax]; \
extern uint T##Max;	   \
extern uint T##Active; \
extern T * T##FFree; \
T * T##AllocRaw(); \
void T##Free(T * v);

#include "allocator-types.h"
#undef defineAllocator

extern lNFunc   lNFuncList[NFN_MAX];
extern uint     lNFuncMax;


lArray *         lArrayAlloc         (size_t len);
lNFunc *         lNFuncAlloc         ();
lBytecodeArray * lBytecodeArrayAlloc (size_t len);
lVal *           lValAlloc           (lType t);
int              lBufferViewTypeSize (lBufferViewType T);
lBuffer *        lBufferAlloc        (size_t length, bool immutable);
lBufferView *    lBufferViewAlloc    (lBuffer *buf, lBufferViewType type, size_t offset, size_t length, bool immutable);

static inline int lClosureID(const lClosure *n){
	return n - lClosureList;
}
static inline int lNFuncID(const lNFunc *n){
	return n - lNFuncList;
}
static inline int lValIndex(const lVal *v){
	return v - lValList;
}
static inline lVal *lIndexVal(uint i){
	return (i >= lValMax) ? NULL : &lValList[i];
}

#endif
