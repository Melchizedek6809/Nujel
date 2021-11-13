#pragma once
#include "../nujel.h"

lClosure *lRootsClosurePush(lClosure *c);
lVal     *lRootsValPush    (lVal *c);
lString  *lRootsStringPush (lString *s);

void      lRootsMark();

extern int rootSP;

static inline void lRootsRet(const int i){ rootSP = i; }
static inline int lRootsGet(){ return rootSP; }

#define RVP(c) lRootsValPush(c)
