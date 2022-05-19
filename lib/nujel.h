#ifndef NUJEL_LIB_NUJEL
#define NUJEL_LIB_NUJEL
#include "common.h"
#include "allocation/allocator.h"
#include "allocation/garbage-collection.h"
#include "allocation/roots.h"

#include <setjmp.h>

extern bool    lVerbose;
extern jmp_buf exceptionTarget;
extern lVal   *exceptionValue;
extern int     exceptionTargetDepth;

void  lExceptionThrowRaw    (lVal *v) NORETURN;
void  lExceptionThrowValClo (const char *symbol, const char *error, lVal *v, lClosure *c) NORETURN;

void      lInit    ();
lClosure *lNewRoot ();
lVal     *lLambda  (lClosure *c, lVal *args, lVal *lambda);
lVal     *lApply   (lClosure *c, lVal *args, lVal *fun);
lClosure *lLoad    (lClosure *c, const char *expr);

lVal *lnfCat     (lClosure *c, lVal *v);
lVal *lnfArrNew  (lClosure *c, lVal *v);
lVal *lnfTreeNew (lClosure *c, lVal *v);
lVal *lnfVec     (lClosure *c, lVal *v);

#endif
