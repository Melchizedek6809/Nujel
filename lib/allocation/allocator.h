#ifndef NUJEL_LIB_ALLOC_ALLOCATOR
#define NUJEL_LIB_ALLOC_ALLOCATOR
#include "../nujel.h"

#define ARR_MAX (1<<14)
#define TRE_MAX (1<<19)
#define CLO_MAX (1<<16)
#define NFN_MAX (1<<10)
#define STR_MAX (1<<14)
#define VAL_MAX (1<<21)
#define BCA_MAX (1<<14)
#define BUF_MAX (1<<14)
#define BFV_MAX (1<<14)

extern lArray  lArrayList[ARR_MAX];
extern uint    lArrayMax;
extern lArray *lArrayFFree;

extern lClosure lClosureList[CLO_MAX];
extern uint     lClosureMax;
extern lClosure *lClosureFFree;

extern lNFunc   lNFuncList  [NFN_MAX];
extern uint     lNFuncMax;
extern uint     lNFuncActive;

extern lTree  lTreeList[TRE_MAX];
extern uint   lTreeMax;
extern lTree *lTreeFFree;

extern lVal     lValList[VAL_MAX];
extern uint     lValMax;
extern lVal    *lValFFree;

extern lBytecodeArray  lBytecodeArrayList[BCA_MAX];
extern uint            lBytecodeArrayMax;
extern lBytecodeArray *lBytecodeArrayFFree;

extern lBuffer  lBufferList[BUF_MAX];
extern uint     lBufferMax;
extern lBuffer *lBufferFFree;

extern lBufferView  lBufferViewList[BUF_MAX];
extern uint         lBufferViewMax;
extern lBufferView *lBufferViewFFree;


lArray *  lArrayAlloc  (size_t len);
void      lArrayFree   (lArray *v);

lClosure *lClosureAlloc();
void      lClosureFree (lClosure *c);
static inline int lClosureID(const lClosure *n){
	return n - lClosureList;
}

lNFunc   *lNFuncAlloc  ();
static inline int lNFuncID(const lNFunc *n){
	return n - lNFuncList;
}

lBytecodeArray *lBytecodeArrayAlloc(size_t len);
void            lBytecodeArrayFree(lBytecodeArray *a);

lTree    *lTreeAlloc   ();
void      lTreeFree    (lTree *t);

lVal     *lValAllocRaw ();
void      lValFree     (lVal *v);
static inline int lValIndex(const lVal *v){
	return v - lValList;
}
static inline lVal *lIndexVal(uint i){
	return (i >= lValMax) ? NULL : &lValList[i];
}
lVal *lValAlloc(lType t);

int      lBufferViewTypeSize(lBufferViewType T);
lBuffer *lBufferAllocRaw();
lBuffer *lBufferAlloc(size_t length, bool immutable);
void     lBufferFree (lBuffer *buf);

lBufferView *lBufferViewAlloc(lBuffer *buf, lBufferViewType type, size_t offset, size_t length, bool immutable);
void         lBufferViewFree (lBufferView *buf);

#endif
