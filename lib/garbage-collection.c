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
defineAllocator(lSymbol, SYM_MAX)
defineAllocator(lNFunc, NFN_MAX)
#undef defineAllocator

#define heapMarkerPrefix(T, chunkBytes) \
if(unlikely(v == NULL)){return;} \
u8 *mark = lHeapMarkByte(v, sizeof(T), chunkBytes); \
if(*mark){return;} \
*mark = 1

#define staticMarkerPrefix(T) \
if(unlikely(v == NULL)){return;} \
const uint ci = v - T##List; \
if(T##MarkMap[ci]){return;} \
T##MarkMap[ci] = 1

#define lHeapFreeMark(v, T, chunkBytes) do { \
	lHeapActiveBytes -= sizeof(T); \
	*lHeapMarkByte((v), sizeof(T), (chunkBytes)) = 2; \
} while(0)

static void lValGCMark         (lVal v);
static void lBufferGCMark      (const lBuffer *v);
static void lBufferViewGCMark  (const lBufferView *v);
static void lMapGCMark         (const lMap *v);
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
	lHeapFreeMark(buf, lBuffer, BUF_CHUNK_BYTES);
}


static void lBufferViewFree(lBufferView *buf){
	buf->nextFree = lBufferViewFFree;
	lBufferViewActive--;
	lBufferViewFFree = buf;
	lHeapFreeMark(buf, lBufferView, BFV_CHUNK_BYTES);
}

static void lBytecodeArrayFree(lBytecodeArray *v){
	if(!(v->flags & BUFFER_STATIC)){
		free(v->data);
	}
	v->nextFree = lBytecodeArrayFFree;
	lBytecodeArrayActive--;
	lBytecodeArrayFFree = v;
	lHeapFreeMark(v, lBytecodeArray, BCA_CHUNK_BYTES);
}

static void lNFuncFree(lNFunc *n){
	(void)n;
}

static void lArrayFree(lArray *v){
	free(v->data);
	v->nextFree = lArrayFFree;
	lArrayFFree = v;
	lArrayActive--;
	lHeapFreeMark(v, lArray, ARR_CHUNK_BYTES);
}

static void lClosureFree(lClosure *clo){
	clo->nextFree = lClosureFFree;
	lClosureFFree = clo;
	lClosureActive--;
	lHeapFreeMark(clo, lClosure, CLO_CHUNK_BYTES);
}

static void lTreeFree(lTree *t){
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
	lTreeActive--;
	lHeapFreeMark(t, lTree, TRE_CHUNK_BYTES);
}

static void lMapFree(lMap *t){
	free(t->entries);
	t->entries = NULL;
	t->nextFree = lMapFFree;
	lMapFFree = t;
	lMapActive--;
	lHeapFreeMark(t, lMap, MAP_CHUNK_BYTES);
}

static void lTreeRootFree(lTreeRoot *t){
	t->nextFree = lTreeRootFFree;
	lTreeRootFFree = t;
	lTreeRootActive--;
	lHeapFreeMark(t, lTreeRoot, TRR_CHUNK_BYTES);
}

static void lPairFree(lPair *cons){
	cons->car = NIL;
	cons->nextFree = lPairFFree;
	lPairFFree = cons;
	lPairActive--;
	lHeapFreeMark(cons, lPair, CON_CHUNK_BYTES);
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
	heapMarkerPrefix(lBuffer, BUF_CHUNK_BYTES);
}

static void lBufferViewGCMark(const lBufferView *v){
	heapMarkerPrefix(lBufferView, BFV_CHUNK_BYTES);

	lBufferGCMark(v->buf);
}

static void lSymbolGCMark(const lSymbol *v){
	staticMarkerPrefix(lSymbol);
}

static void lNFuncGCMark(const lNFunc *v){
	staticMarkerPrefix(lNFunc);

	lTreeGCMark(v->meta);
}

static void lPairGCMark(const lPair *v){
	heapMarkerPrefix(lPair, CON_CHUNK_BYTES);

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
	case ltMap:
		lMapGCMark(v.vMap);
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

static void lMapGCMark(const lMap *v){
	heapMarkerPrefix(lMap, MAP_CHUNK_BYTES);
	for(int i=0;i<v->size;i++){
		if(v->entries[i].key.type == ltNil){continue;}
		lValGCMark(v->entries[i].key);
		lValGCMark(v->entries[i].val);
	}
}

static void lTreeGCMark(const lTree *v){
	heapMarkerPrefix(lTree, TRE_CHUNK_BYTES);

	lSymbolGCMark(v->key);
	lValGCMark(v->value);

	lTreeGCMark(v->left);
	lTreeGCMark(v->right);
}

static void lTreeRootGCMark(const lTreeRoot *v){
	heapMarkerPrefix(lTreeRoot, TRR_CHUNK_BYTES);
	lTreeGCMark(v->root);
}

static void lClosureGCMark(const lClosure *v){
	heapMarkerPrefix(lClosure, CLO_CHUNK_BYTES);

	lClosureGCMark(v->parent);
	lTreeGCMark(v->data);
	lTreeGCMark(v->meta);
	lBytecodeArrayMark(v->text);
	lValGCMark(v->args);
}

static void lArrayGCMark(const lArray *v){
	heapMarkerPrefix(lArray, ARR_CHUNK_BYTES);

	for(int i=0;i<v->length;i++){
		lValGCMark(v->data[i]);
	}
}

static void lBytecodeArrayMark(const lBytecodeArray *v){
	heapMarkerPrefix(lBytecodeArray, BCA_CHUNK_BYTES);

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
	lMapGCMark(lSymbolTable);
}

lSymbol *lRootsSymbolPush(lSymbol *v){
	lSymbolMarkMap[v - lSymbolList] = 2;
	return v;
}

/* Free all values that have not been marked by lGCMark */
static void lGCSweep(){
	#define defineAllocator(T, typeTag, chunkBytes) \
	for(uint bi=0;bi < lHeapBlockMax;bi++){\
		lHeapBlock *block = &lHeapBlocks[bi];\
		if(block->type != typeTag){continue;}\
		T *objects = block->ptr;\
		u8 *marks = ((u8 *)block->ptr) + block->size;\
		const uint count = block->size / sizeof(T);\
		for(uint i=0;i < count;i++){\
			if(marks[i] == 0) {\
				T##Free(&objects[i]);\
			}\
			marks[i] = marks[i]&2;\
		}\
	}
	allocatorTypes()
	#undef defineAllocator

	#define defineStaticAllocator(T, TMAX) \
	for(uint i=0;i < T##Max;i++){\
		if(T##MarkMap[i] == 0) {\
			T##Free(&T##List[i]);\
		}\
		T##MarkMap[i] = T##MarkMap[i]&2;\
	}
	defineStaticAllocator(lSymbol, SYM_MAX)
	defineStaticAllocator(lNFunc, NFN_MAX)
	#undef defineStaticAllocator
}

static void lHeapUpdateGCThreshold(){
	const size_t minThreshold = 4 * 1024 * 1024;
	const size_t doubled = lHeapActiveBytes * 2;
	lHeapNextGCBytes = MAX(minThreshold, doubled);
}

/* Force a garbage collection cycle, shouldn't need to be called manually since
 * when the heap is exhausted the GC is run */
void lGarbageCollect(lThread *ctx){
	lGCRuns++;
	lRootsMark();
	lThreadGCMark(ctx);

	lGCSweep();
	lHeapUpdateGCThreshold();
	lGCShouldRunSoon = false;
}
