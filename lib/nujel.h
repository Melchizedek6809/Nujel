#ifndef NUJEL_LIB_NUJEL
#define NUJEL_LIB_NUJEL
#include "common.h"
#include "allocation/array.h"
#include "allocation/closure.h"
#include "allocation/garbage-collection.h"
#include "allocation/roots.h"
#include "allocation/string.h"
#include "allocation/val.h"

extern bool lVerbose;

extern lVal *lnfvQuote;

void      lInit             ();
lClosure *lNewRoot          ();
lVal     *lMap              (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lEval             (lClosure *c, lVal *v);
lVal     *lLambda           (lClosure *c, lVal *args, lVal *lambda);
lVal     *lApply            (lClosure *c, lVal *args, lVal *fun, lVal *funSym);
lClosure *lLoad             (lClosure *c, const char *expr);
void      lBreak            ();

#endif
