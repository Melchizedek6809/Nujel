#pragma once
#include "common.h"

typedef struct lArray   lArray;;
typedef struct lClosure lClosure;
typedef struct lNFunc   lNFunc;;
typedef struct lSymbol  lSymbol;
typedef struct lString  lString;;
typedef struct lTree    lTree;
typedef struct lVec     lVec;
typedef struct lVal     lVal;

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

lVal *lRootsValPush         (lVal *c);
lVal *lRootsValPop          ();
