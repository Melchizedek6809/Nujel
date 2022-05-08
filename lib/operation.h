#ifndef NUJEL_LIB_OPERATION
#define NUJEL_LIB_OPERATION
#include "nujel.h"

lVal *lnfCat     (lClosure *c, lVal *v);
lVal *lnfArrNew  (lClosure *c, lVal *v);
lVal *lnfTreeNew (lClosure *c, lVal *v);
lVal *lnfVec     (lClosure *c, lVal *v);

void lAddCoreFuncs(lClosure *c);

#endif
