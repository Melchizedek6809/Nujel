#ifndef NUJEL_LIB_NUJEL
#define NUJEL_LIB_NUJEL
#include "common.h"
#include "allocation/allocator.h"
#include "allocation/garbage-collection.h"
#include "allocation/roots.h"

extern bool lVerbose;

void      lInit    ();
lClosure *lNewRoot ();
lVal     *lLambda  (lClosure *c, lVal *args, lVal *lambda);
lVal     *lApply   (lClosure *c, lVal *args, lVal *fun);
lClosure *lLoad    (lClosure *c, const char *expr);

#endif
