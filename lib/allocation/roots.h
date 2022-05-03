#ifndef NUJEL_LIB_ALLOC_ROOTS
#define NUJEL_LIB_ALLOC_ROOTS
#include "../nujel.h"

lClosure *lRootsClosurePush   (lClosure *c);
lVal     *lRootsValPush       (lVal *c);
lTree    *lRootsTreePush      (lTree *c);
lString  *lRootsStringPush    (lString *s);
lSymbol  *lRootsSymbolPush    (lSymbol *s);
void      lRootsBytecodePush  (lVal *start);
void      lRootsThreadPush    (lThread *ctx);


void      lRootsMark();

extern int rootSP;
extern void (*rootsMarkerChain)();

static inline void lRootsRet(const int i){ rootSP = i; }
static inline int  lRootsGet(){ return rootSP; }

#define RVP(c)   lRootsValPush(c)
#define RCP(c)   lRootsClosurePush(c)
#define RSP(c)   lRootsStringPush(c)
#define RSYMP(c) lRootsSymbolPush(c)
#define RTP(c)   lRootsTreePush(c)

#endif
