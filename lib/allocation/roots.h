#ifndef NUJEL_LIB_ALLOC_ROOTS
#define NUJEL_LIB_ALLOC_ROOTS
#include "../nujel.h"

lVal     *lRootsValPush       (lVal *c);
void      lRootsMark();

extern int rootSP;
extern void (*rootsMarkerChain)();

static inline void lRootsRet(const int i){ rootSP = i; }
static inline int  lRootsGet(){ return rootSP; }

#define RVP(c)   lRootsValPush(c)

#endif
