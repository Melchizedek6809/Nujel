/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */

/*
 * Contains a terrible implementation of a mark-sweep garbage collector, but it
 * is good enough for now.
 */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

#include <stdlib.h>

typedef struct {
	lType t;
	union {
		lClosure *vClosure;
		lSymbol  *vSymbol;
		void     *vPointer;
		lThread  *vThread;
	};
} rootEntry;

rootEntry *rootStack = NULL;
int rootSP  = 0;
int rootMax = 0;
int lGCRuns = 0;

#define defineAllocator(T, TMAX) u8 T##MarkMap[TMAX];
allocatorTypes()
defineAllocator(lSymbol, SYM_MAX)
defineAllocator(lNFunc, NFN_MAX)
#undef defineAllocator

#define markerPrefix(T) \
if(unlikely(v == NULL)){return;} \
const uint ci = v - T##List; \
if(unlikely(ci >= T##Max)){return;} \
if(T##MarkMap[ci]){return;} \
T##MarkMap[ci] = 1						\

void lThreadGCMark(lThread *c){
	if(unlikely(c == NULL)){return;}
	if(unlikely((c->csp > 8192) || (c->csp < 0))){
		epf("Ignoring closure due to strangely sized CSP: %i\n", (i64)c->csp);
		return;
	}
	lBytecodeArrayMark(c->text);
	for(int i=0;i <= c->csp;i++){
		lClosureGCMark(c->closureStack[i]);
	}
	for(int i=0;i < c->sp;i++){
		lValGCMark(c->valueStack[i]);
	}
}

void lBufferGCMark(const lBuffer *v){
	markerPrefix(lBuffer);
}

void lBufferViewGCMark(const lBufferView *v){
	markerPrefix(lBufferView);

	lBufferGCMark(lBufferViewList[ci].buf);
}

void lSymbolGCMark(const lSymbol *v){
	markerPrefix(lSymbol);
}

void lNFuncGCMark(const lNFunc *v){
	markerPrefix(lNFunc);

	lSymbolGCMark(v->name);
	lValGCMark(v->args);
	lTreeGCMark(v->meta);
}

void lValGCMark(lVal *v){
	markerPrefix(lVal);

	switch(v->type){
	case ltPair:
		lValGCMark(v->vList.car);
		lValGCMark(v->vList.cdr);
		break;
	case ltMacro:
	case ltObject:
	case ltLambda:
		lClosureGCMark(v->vClosure);
		break;
	case ltArray:
		lArrayGCMark(v->vArray);
		break;
	case ltNativeFunc:
		lNFuncGCMark(v->vNFunc);
		break;
	case ltKeyword:
	case ltSymbol:
		lSymbolGCMark(v->vSymbol);
		break;
	case ltTree:
		lTreeGCMark(v->vTree);
		break;
	case ltBytecodeArr:
		lBytecodeArrayMark(v->vBytecodeArr);
		break;
	case ltString:
	case ltBuffer:
		lBufferGCMark(v->vBuffer);
		break;
	case ltBufferView:
		lBufferViewGCMark(v->vBufferView);
		break;
	default:
		break;
	}
}

void lTreeGCMark(const lTree *v){
	markerPrefix(lTree);

	lSymbolGCMark(v->key);
	lValGCMark(v->value);
	lTreeGCMark(v->left);
	lTreeGCMark(v->right);
}

void lClosureGCMark(const lClosure *v){
	markerPrefix(lClosure);

	lClosureGCMark(v->parent);
	lTreeGCMark(v->data);
	lTreeGCMark(v->meta);
	lBytecodeArrayMark(v->text);
	lValGCMark(v->args);
	lSymbolGCMark(v->name);
	lClosureGCMark(v->caller);
}

void lArrayGCMark(const lArray *v){
	markerPrefix(lArray);

	for(int i=0;i<v->length;i++){
		lValGCMark(v->data[i]);
	}
}

void lBytecodeArrayMark(const lBytecodeArray *v){
	markerPrefix(lBytecodeArray);

	lArrayGCMark(v->literals);
}

/* There should be a way to avoid having this procedure alltogether, but for
 * now a solution is not apparent to me. It marks every free object so it won't
 * get freed again.
 */
static void lMarkFree(){
	#define defineAllocator(T, TMAX)\
	for(T *v = T##FFree;v;v=v->nextFree){\
		T##MarkMap[v - T##List] = 1;\
	}
	allocatorTypes()
	defineAllocator(lSymbol, SYM_MAX)
	#undef defineAllocator
}

static void lMarkNFuncs(){
	for(uint i=0;i<lNFuncMax;i++){
		lNFuncGCMark(&lNFuncList[i]);
	}
}

/* Mark every single root and everything they point to */
static void lRootsMark(){
	for(int i=0;i<rootSP;i++){
		switch(rootStack[i].t){
		case ltSymbol:
			lSymbolGCMark(rootStack[i].vSymbol);
			break;
		case ltLambda:
			lClosureGCMark(rootStack[i].vClosure);
			break;
		case ltThread:
			lThreadGCMark(rootStack[i].vThread);
			break;
		default:
			break;
		}
	}
}

static void *lRootsPush(const lType t, void *ptr){
	if(unlikely(rootSP >= rootMax)){
		rootMax = MAX(rootMax * 2, 256);
		rootEntry *newRootStack = realloc(rootStack, rootMax * sizeof(rootEntry));
		if(unlikely(newRootStack == NULL)){
			free(rootStack);
			epf("Can't grow rootStack\n");
			exit(123);
		}
		rootStack = newRootStack;
	}
	rootStack[rootSP].t = t;
	rootStack[rootSP].vPointer = ptr;
	rootSP++;
	return ptr;
}

lClosure *lRootsClosurePush(lClosure *v){
	return lRootsPush(ltLambda, v);
}
lSymbol *lRootsSymbolPush(lSymbol *v){
	return lRootsPush(ltSymbol, v);
}
lThread *lRootsThreadPush(lThread *v){
	return lRootsPush(ltThread, v);
}

/* Mark the roots so they will be skipped by the GC,  */
static void lGCMark(){
	lRootsMark();
	lMarkNFuncs();
	lMarkFree();
}

/* Free all values that have not been marked by lGCMark */
static void lGCSweep(){
	#define defineAllocator(T, TMAX) \
	for(uint i=0;i < T##Max;i++){\
		if(T##MarkMap[i]){\
			T##MarkMap[i]=0;\
		}else{\
			T##Free(&T##List[i]);\
		}\
	}
	allocatorTypes()
	defineAllocator(lSymbol, SYM_MAX)
	defineAllocator(lNFunc, NFN_MAX)
	#undef defineAllocator
}

/* Force a garbage collection cycle, shouldn't need to be called manually since
 * when the heap is exhausted the GC is run */
void lGarbageCollect(){
	lGCMark();
	lGCSweep();
	lGCShouldRunSoon = false;
}
