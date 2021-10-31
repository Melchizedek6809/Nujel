#pragma once
#include "common.h"
#include "allocation/array.h"
#include "allocation/closure.h"
#include "allocation/garbage-collection.h"
#include "allocation/roots.h"
#include "allocation/string.h"
#include "allocation/val.h"

extern bool lVerbose;

extern lVal *lnfvInfix;
extern lVal *lnfvArrRef;
extern lVal *lnfvCat;
extern lVal *lnfvTreeGet;

void      lInit             ();

lClosure *lClosureNewRoot   ();
lClosure *lClosureNewRootNoStdLib();

lVal     *lMap              (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lEval             (lClosure *c, lVal *v);
lVal     *lApply            (lClosure *c, lVal *args, lVal *fun);

lVal     *lWrap             (lVal *v);
