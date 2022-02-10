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

extern lVal *lnfvDo;
extern lVal *lnfvInfix;
extern lVal *lnfvArrRef;
extern lVal *lnfvCat;
extern lVal *lnfvTreeGet;
extern lVal *lnfvQuote;

void      lInit             ();

lClosure *lNewRoot          ();
lClosure *lNewRootNoStdLib  ();

lVal     *lRun              (lClosure *c, lVal *v);
lVal     *lRunS             (lClosure *c, const char *s, int sLen);
void      lLoad             (lClosure *c, lVal *v);
void      lLoadS            (lClosure *c, const char *s, int sLen);
lVal     *lMap              (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lEval             (lClosure *c, lVal *v);
lVal     *lMacro            (lClosure *c, lVal *args, lVal *lambda);
lVal     *lLambda           (lClosure *c, lVal *args, lVal *lambda);
lVal     *lApply            (lClosure *c, lVal *args, lVal *fun, lVal *funSym);
lVal     *lTry              (lClosure *c, lVal *catchRaw, lVal *bodyRaw);
lVal     *lQuote            (lVal *v);
void      lBreak            ();

#endif
