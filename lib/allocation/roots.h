#pragma once
#include "../nujel.h"

lClosure *lRootsClosurePush(lClosure *c);
lVal     *lRootsValPush    (lVal *c);
lTree    *lRootsTreePush   (lTree *c);
lString  *lRootsStringPush (lString *s);

void      lRootsMark();

extern int rootSP;
extern void (*rootsMarkerChain)();

static inline void lRootsRet(const int i){ rootSP = i; }
static inline int lRootsGet(){ return rootSP; }

#define RVP(c) lRootsValPush(c)
#define RCP(c) lRootsClosurePush(c)
#define RSP(c) lRootsStringPush(c)
#define RTP(c) lRootsTreePush(c)
