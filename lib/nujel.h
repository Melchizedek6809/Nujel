#pragma once
#include "common.h"
#include "allocation/roots.h"

extern bool lVerbose;

void      lInit             ();
int       lMemUsage         ();
void      lPrintError       (const char *format, ...);

lClosure *lClosureNewRoot   ();
lClosure *lClosureNewRootNoStdLib();

void      lDisplayVal       (lVal *v);
void      lDisplayErrorVal  (lVal *v);
void      lWriteVal         (lVal *v);
void      lWriteTree        (lTree *t);

lVal     *lMap              (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lEval             (lClosure *c, lVal *v);
lVal     *lWrap             (lVal *v);
