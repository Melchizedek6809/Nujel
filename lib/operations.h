#ifndef NUJEL_LIB_OPERATIONS
#define NUJEL_LIB_OPERATIONS
#include "common.h"

void lOperationsBase(lClosure *c);

lVal *lAdd(lClosure *c, lVal *a, lVal *b);
lVal *lSub(lClosure *c, lVal *a, lVal *b);
lVal *lMul(lClosure *c, lVal *a, lVal *b);
lVal *lDiv(lClosure *c, lVal *a, lVal *b);
lVal *lRem(lClosure *c, lVal *a, lVal *b);

#endif
