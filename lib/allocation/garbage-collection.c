/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */

/*
 * Contains a terrible implementation of a mark-sweep garbage collector, but it
 * is good enough for now.
 */
#include "garbage-collection.h"

#include "bytecode.h"
#include "allocator.h"
#include "roots.h"
#include "symbol.h"
#include "../printer.h"
#include "../compatibility/builtins.h"

#include <stdlib.h>

int lGCRuns = 0;

#define defineAllocator(T, TMAX) u8 T##MarkMap[TMAX];
#include "allocator-types.h"
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

	lTreeGCMark(v->meta);
	lValGCMark(v->args);
	lSymbolGCMark(v->name);
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
	case ltGUIWidget:
		lWidgetMarkI(v->vInt);
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
	lBytecodeArrayMarkRefs(v);
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
	#include "allocator-types.h"
	defineAllocator(lSymbol, SYM_MAX)
	#undef defineAllocator
}

/* Mark the roots so they will be skipped by the GC,  */
static void lGCMark(){
	lRootsMark();
	lMarkFree();
}

void (*sweeperChain)() = NULL;
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
	#include "allocator-types.h"
	defineAllocator(lSymbol, SYM_MAX)
	#undef defineAllocator
	if(sweeperChain != NULL){sweeperChain();}
}

/* Force a garbage collection cycle, shouldn't need to be called manually since
 * when the heap is exhausted the GC is run */
void lGarbageCollect(){
	//pf("GC!\n");
	lGCRuns++;
	lGCMark();
	lGCSweep();
	lGCShouldRunSoon = false;
}
