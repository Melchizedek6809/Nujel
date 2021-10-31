#pragma once
#include "common.h"
#include "allocation/roots.h"

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

void      lPrintError       (const char *format, ...);
void      lDisplayVal       (lVal *v);
void      lDisplayErrorVal  (lVal *v);
void      lWriteVal         (lVal *v);
void      lWriteTree        (lTree *t);
