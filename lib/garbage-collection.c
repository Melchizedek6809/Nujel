/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */

/*
 * Contains a terrible implementation of a mark-sweep garbage collector, but it
 * is good enough for now.
 */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

int lGCRuns = 0;

u8 fileDescriptorMarkMap[MAX_OPEN_FILE_DESCRIPTORS];

#define defineAllocator(T, TMAX) u8 T##MarkMap[TMAX];
allocatorTypes()
defineAllocator(lSymbol, SYM_MAX)
defineAllocator(lNFunc, NFN_MAX)
#undef defineAllocator

#define markerPrefix(T) \
if(unlikely(v == NULL)){return;} \
const uint ci = v - T##List; \
if(T##MarkMap[ci]){return;} \
T##MarkMap[ci] = 1

static void lValGCMark         (lVal v);
static void lBufferGCMark      (const lBuffer *v);
static void lBufferViewGCMark  (const lBufferView *v);
static void lTreeGCMark        (const lTree *v);
static void lTreeRootGCMark    (const lTreeRoot *v);
static void lClosureGCMark     (const lClosure *c);
static void lArrayGCMark       (const lArray *v);
static void lNFuncGCMark       (const lNFunc *f);
static void lSymbolGCMark      (const lSymbol *v);
static void lThreadGCMark      (lThread *c);
static void lBytecodeArrayMark (const lBytecodeArray *v);


static void lBufferFree(lBuffer *buf){
	if(!(buf->flags & BUFFER_STATIC)){
		free(buf->buf);
	}
	buf->nextFree = lBufferFFree;
	lBufferActive--;
	lBufferFFree = buf;
	lBufferMarkMap[buf - lBufferList] = 2;
}


static void lBufferViewFree(lBufferView *buf){
	buf->nextFree = lBufferViewFFree;
	lBufferViewActive--;
	lBufferViewFFree = buf;
	lBufferViewMarkMap[buf - lBufferViewList] = 2;
}

static void lBytecodeArrayFree(lBytecodeArray *v){
	if(!(v->flags & BUFFER_STATIC)){
		free(v->data);
	}
	v->nextFree = lBytecodeArrayFFree;
	lBytecodeArrayActive--;
	lBytecodeArrayFFree = v;
	lBytecodeArrayMarkMap[v - lBytecodeArrayList] = 2;
}

static void lNFuncFree(lNFunc *n){
	(void)n;
}

static void lArrayFree(lArray *v){
	free(v->data);
	v->nextFree = lArrayFFree;
	lArrayFFree = v;
	lArrayActive--;
	lArrayMarkMap[v - lArrayList] = 2;
}

static void lClosureFree(lClosure *clo){
	clo->nextFree = lClosureFFree;
	lClosureFFree = clo;
	lClosureActive--;
	lClosureMarkMap[clo - lClosureList] = 2;
}

static void lTreeFree(lTree *t){
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
	lTreeActive--;
	lTreeMarkMap[t - lTreeList] = 2;
}

static void lTreeRootFree(lTreeRoot *t){
	t->nextFree = lTreeRootFFree;
	lTreeRootFFree = t;
	lTreeRootActive--;
	lTreeRootMarkMap[t - lTreeRootList] = 2;
}

static void lPairFree(lPair *cons){
	cons->car = NIL;
	cons->nextFree = lPairFFree;
	lPairFFree = cons;
	lPairActive--;
	lPairMarkMap[cons - lPairList] = 2;
}

static void lThreadGCMark(lThread *c){
	lBytecodeArrayMark(c->text);
	for(int i=0;i <= c->csp;i++){
		lClosureGCMark(c->closureStack[i]);
	}
	for(int i=0;i <= c->sp;i++){
		lValGCMark(c->valueStack[i]);
	}
}

static void lBufferGCMark(const lBuffer *v){
	markerPrefix(lBuffer);
}

static void lBufferViewGCMark(const lBufferView *v){
	markerPrefix(lBufferView);

	lBufferGCMark(lBufferViewList[ci].buf);
}

static void lSymbolGCMark(const lSymbol *v){
	markerPrefix(lSymbol);
}

static void lNFuncGCMark(const lNFunc *v){
	markerPrefix(lNFunc);

	lValGCMark(v->args);
	lTreeGCMark(v->meta);
}

static void lPairGCMark(const lPair *v){
	markerPrefix(lPair);

	lValGCMark(v->car);
	lValGCMark(v->cdr);
}

static void lValGCMark(lVal v){
	switch(v.type){
	case ltPair:
		lPairGCMark(v.vList);
		break;
	case ltMacro:
	case ltEnvironment:
	case ltLambda:
		lClosureGCMark(v.vClosure);
		break;
	case ltArray:
		lArrayGCMark(v.vArray);
		break;
	case ltNativeFunc:
		lNFuncGCMark(v.vNFunc);
		break;
	case ltKeyword:
	case ltSymbol:
		lSymbolGCMark(v.vSymbol);
		break;
	case ltTree:
		lTreeRootGCMark(v.vTree);
		break;
	case ltBytecodeArr:
		lBytecodeArrayMark(v.vBytecodeArr);
		break;
	case ltString:
	case ltBuffer:
		lBufferGCMark(v.vBuffer);
		break;
	case ltBufferView:
		lBufferViewGCMark(v.vBufferView);
		break;
	default:
		break;
	}
}

static void lTreeGCMark(const lTree *v){
	markerPrefix(lTree);

	lSymbolGCMark(v->key);
	lValGCMark(v->value);

	lTreeGCMark(v->left);
	lTreeGCMark(v->right);
}

static void lTreeRootGCMark(const lTreeRoot *v){
	markerPrefix(lTreeRoot);
	lTreeGCMark(v->root);
}

static void lClosureGCMark(const lClosure *v){
	markerPrefix(lClosure);

	lClosureGCMark(v->parent);
	lTreeGCMark(v->data);
	lTreeGCMark(v->meta);
	lBytecodeArrayMark(v->text);
	lValGCMark(v->args);
}

static void lArrayGCMark(const lArray *v){
	markerPrefix(lArray);

	for(int i=0;i<v->length;i++){
		lValGCMark(v->data[i]);
	}
}

static void lBytecodeArrayMark(const lBytecodeArray *v){
	markerPrefix(lBytecodeArray);

	lArrayGCMark(v->literals);
}

/* Mark every single root and everything they point to */
static void lRootsMark(){
	for(size_t i=0;i < countof(lClassList);i++){
		const lClass *T = &lClassList[i];
		lSymbolGCMark(T->name);
		lTreeGCMark(T->methods);
		lTreeGCMark(T->staticMethods);
	}
	for(uint i=0;i<lNFuncMax;i++){
		lNFuncGCMark(&lNFuncList[i]);
	}
}

lSymbol *lRootsSymbolPush(lSymbol *v){
	lSymbolMarkMap[v - lSymbolList] = 2;
	return v;
}

/* Free all values that have not been marked by lGCMark */
static void lGCSweep(){
	#define defineAllocator(T, TMAX) \
	for(uint i=0;i < T##Max;i++){\
		if(T##MarkMap[i] == 0) {\
			T##Free(&T##List[i]);\
		}\
		T##MarkMap[i] = T##MarkMap[i]&2;\
	}
	allocatorTypes()
	defineAllocator(lSymbol, SYM_MAX)
	defineAllocator(lNFunc, NFN_MAX)
	#undef defineAllocator
}

/* Force a garbage collection cycle, shouldn't need to be called manually since
 * when the heap is exhausted the GC is run */
void lGarbageCollect(lThread *ctx){
	lGCRuns++;
	lRootsMark();
	lThreadGCMark(ctx);

	lGCSweep();
	lGCShouldRunSoon = false;
}
