#ifndef NUJEL_LIB_ALLOC_ROOTS
#define NUJEL_LIB_ALLOC_ROOTS
#include "../nujel.h"

lVal     *lRootsValPush    (lVal *c);
lClosure *lRootsClosurePush(lClosure *v);
lSymbol  *lRootsSymbolPush (lSymbol *v);
lThread  *lRootsThreadPush (lThread *v);
void      lRootsMark       ();

extern int rootSP;
extern void (*rootsMarkerChain)();

static inline void lRootsRet(const int i){ rootSP = i; }
static inline int  lRootsGet(){ return rootSP; }

#endif
