#ifndef NUJEL_LIB_ALLOC_GC
#define NUJEL_LIB_ALLOC_GC

#include "../nujel.h"

extern int lGCRuns;
extern void (*sweeperChain)();

void lWidgetMarkI       (uint i);

void lValGCMark         (lVal *v);
void lTreeGCMark        (const lTree *v);
void lClosureGCMark     (const lClosure *c);
void lStringGCMark      (const lString *v);
void lArrayGCMark       (const lArray *v);
void lNFuncGCMark       (const lNFunc *f);
void lSymbolGCMark      (const lSymbol *v);
void lContextGCMark     (lContext *c);

void lGarbageCollect();

#endif
