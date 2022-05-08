#ifndef NUJEL_LIB_ALLOC_ALLOCATOR
#define NUJEL_LIB_ALLOC_ALLOCATOR
#include "../nujel.h"

#define ARR_MAX (1<<12)
#define TRE_MAX (1<<19)
#define CLO_MAX (1<<16)
#define NFN_MAX (1<<10)
#define STR_MAX (1<<14)
#define VAL_MAX (1<<21)


extern lArray  lArrayList[ARR_MAX];
extern uint    lArrayMax;
extern lArray *lArrayFFree;

extern lClosure lClosureList[CLO_MAX];
extern uint     lClosureMax;
extern lClosure *lClosureFFree;

extern lNFunc   lNFuncList  [NFN_MAX];
extern uint     lNFuncMax;
extern uint     lNFuncActive;

extern lString  lStringList [STR_MAX];
extern uint     lStringMax;
extern lString *lStringFFree;

extern lTree  lTreeList[TRE_MAX];
extern uint   lTreeMax;
extern lTree *lTreeFFree;

extern lVal     lValList[VAL_MAX];
extern uint     lValMax;
extern lVal    *lValFFree;


lArray   *lArrayAlloc  ();
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

lString  *lStringAlloc ();
void      lStringFree  (lString *s);

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
static inline lVal *lValAlloc(lType t){
	lVal *ret = lValAllocRaw();
	ret->type = t;
	return ret;
}

#endif
