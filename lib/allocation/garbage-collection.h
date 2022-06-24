#ifndef NUJEL_LIB_ALLOC_GC
#define NUJEL_LIB_ALLOC_GC

#include "../nujel.h"

extern bool lGCShouldRunSoon;
extern int lGCRuns;
extern void (*sweeperChain)();

void lWidgetMarkI       (uint i);

void lValGCMark         (lVal *v);
void lBufferGCMark      (const lBuffer *v);
void lBufferViewGCMark  (const lBufferView *v);
void lTreeGCMark        (const lTree *v);
void lClosureGCMark     (const lClosure *c);
void lArrayGCMark       (const lArray *v);
void lNFuncGCMark       (const lNFunc *f);
void lSymbolGCMark      (const lSymbol *v);
void lThreadGCMark      (lThread *c);
void lBytecodeArrayMark (const lBytecodeArray *v);

void lGarbageCollect();
static inline void lGarbageCollectIfNecessary(){
	if(lGCShouldRunSoon){
		lGarbageCollect();
	}
}

#endif
