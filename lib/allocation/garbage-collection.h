#ifndef NUJEL_LIB_ALLOC_GC
#define NUJEL_LIB_ALLOC_GC

#include "../nujel.h"

extern volatile bool breakQueued;
extern int lGCRuns;
extern void (*sweeperChain)();

void lWidgetMarkI  (uint i);

void lValGCMark    (lVal *v);
void lTreeGCMark   (const lTree *v);
void lClosureGCMark(const lClosure *c);
void lStringGCMark (const lString *v);
void lArrayGCMark  (const lArray *v);
void lNFuncGCMark  (const lNFunc *f);
void lSymbolGCMark (const lSymbol *v);
void lBytecodeStackMark(lVal **v);

void lGarbageCollect();

#endif
