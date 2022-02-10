#ifndef NUJEL_LIB_ALLOC_VAL
#define NUJEL_LIB_ALLOC_VAL
#include "../nujel.h"

#define VAL_MAX (1<<21)
extern lVal  lValList[VAL_MAX];
extern uint  lValMax;
extern uint  lValActive;
extern lVal *lValFFree;

void      lValInit  ();
lVal     *lValAlloc ();
void      lValFree  (lVal *v);

static inline int lValIndex(const lVal *v){return v - lValList;}
static inline lVal *lIndexVal(uint i){return (i >= lValMax) ? NULL : &lValList[i];}

#endif
