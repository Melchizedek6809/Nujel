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
#include "../misc/pf.h"

#include <stdlib.h>

int lGCRuns = 0;

u8 lValMarkMap           [VAL_MAX];
u8 lTreeMarkMap          [TRE_MAX];
u8 lClosureMarkMap       [CLO_MAX];
u8 lArrayMarkMap         [ARR_MAX];
u8 lStringMarkMap        [STR_MAX];
u8 lSymbolMarkMap        [SYM_MAX];
u8 lBytecodeArrayMarkMap [BCA_MAX];

void lThreadGCMark(lThread *c){
	if(c == NULL){return;}
	if((c->csp > 8192) || (c->csp < 0)){
		epf("Unlikely csp: %u\n", (i64)c->csp);
		exit(1);
	}
	for(int i=0;i<=c->csp;i++){
		lClosureGCMark(c->closureStack[i]);
	}
	for(int i=0;i<c->sp;i++){
		lValGCMark(c->valueStack[i]);
	}
}

/* Mark v as being in use so it won't get freed by the GC */
void lStringGCMark(const lString *v){
	if(v == NULL){return;}
	const uint ci = v - lStringList;
	if(ci > lStringMax){
		epf("Tried to mark invalid lString\n");
		exit(1);
	}
	if(lStringMarkMap[ci]){return;}
	lStringMarkMap[ci] = 1;
}

/* Mark v as being in use so it won't get freed by the GC */
void lSymbolGCMark(const lSymbol *v){
	if(v == NULL){return;}
	const uint ci = v - lSymbolList;
	if(ci > lSymbolMax){
		epf("Tried to mark invalid lSymbol\n");
		exit(1);
	}
	if(lSymbolMarkMap[ci]){return;}
	lSymbolMarkMap[ci] = 1;
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lValGCMark(lVal *v){
	if(v == NULL){return;}
	const uint ci = v - lValList;
	if(ci > lValMax){
		epf("Tried to mark out of bounds lVal\n");
		exit(1);
	}
	if(lValMarkMap[ci]){return;}
	lValMarkMap[ci] = 1;

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
		lValGCMark(v->vNFunc->doc);
		lValGCMark(v->vNFunc->args);
		lSymbolGCMark(v->vNFunc->name);
		break;
	case ltString:
		lStringGCMark(v->vString);
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
	default:
		break;
	}
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lTreeGCMark(const lTree *v){
	if(v == NULL){return;}
	const uint ci = v - lTreeList;
	if(ci > lTreeMax){
		epf("Tried to mark invalid lTree\n");
		exit(1);
	}
	if(lTreeMarkMap[ci]){return;}
	lTreeMarkMap[ci] = 1;
	lTreeGCMark(v->left);
	lTreeGCMark(v->right);
	lSymbolGCMark(v->key);
	lValGCMark(v->value);
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lClosureGCMark(const lClosure *c){
	if(c == NULL){return;}
	const uint ci = c - lClosureList;
	if(ci >= lClosureMax){
		epf("Tried to mark out of bounds lClosure: %i\n", (i64)ci);
		exit(1);
	}
	if(lClosureMarkMap[ci]){return;}
	lClosureMarkMap[ci] = 1;

	lClosureGCMark(c->parent);
	lClosureGCMark(c->caller);
	lTreeGCMark(c->data);
	lTreeGCMark(c->meta);
	lValGCMark(c->args);
	lBytecodeArrayMark(c->text);
	lSymbolGCMark(c->name);
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lArrayGCMark(const lArray *v){
	if(v == NULL){return;}
	const uint ci = v - lArrayList;
	if(ci > lArrayMax){
		epf("Tried to mark invalid lArray\n");
		exit(1);
	}
	if(lArrayMarkMap[ci]){return;}
	lArrayMarkMap[ci] = 1;
	for(int i=0;i<v->length;i++){
		lValGCMark(v->data[i]);
	}
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lBytecodeArrayMark(const lBytecodeArray *v){
	if(v == NULL){return;}
	const uint ci = v - lBytecodeArrayList;
	if(ci > lBytecodeArrayMax){
		epf("Tried to mark invalid lBytecodeArray\n");
		exit(1);
	}
	if(lBytecodeArrayMarkMap[ci]){return;}
	lBytecodeArrayMarkMap[ci] = 1;
	lBytecodeArrayMarkRefs(v);
}

/* There should be a way to avoid having this procedure alltogether, but for
 * now a solution is not apparent to me. It marks every free object so it won't
 * get freed again.
 */
static void lMarkFree(){
	for(lArray *arr = lArrayFFree;arr != NULL;arr = arr->nextFree){
		const uint ci = arr - lArrayList;
		lArrayMarkMap[ci] = 1;
	}
	for(lClosure *clo = lClosureFFree;clo != NULL;clo = clo->nextFree){
		const uint ci = clo - lClosureList;
		lClosureMarkMap[ci] = 1;
	}
	for(lSymbol *sym = lSymbolFFree;sym != NULL;sym = sym->nextFree){
		const uint ci = sym - lSymbolList;
		lSymbolMarkMap[ci] = 1;
	}
	for(lString *str = lStringFFree;str != NULL;str = str->nextFree){
		const uint ci = str - lStringList;
		lStringMarkMap[ci] = 1;
	}
	for(lVal *v = lValFFree;v != NULL;v = v->nextFree){
		const uint ci = v - lValList;
		lValMarkMap[ci] = 1;
	}
	for(lTree *t = lTreeFFree;t != NULL;t = t->nextFree){
		const uint ci = t - lTreeList;
		lTreeMarkMap[ci] = 1;
	}
	for(lBytecodeArray *t = lBytecodeArrayFFree;t != NULL;t = t->nextFree){
		const uint ci = t - lBytecodeArrayList;
		lBytecodeArrayMarkMap[ci] = 1;
	}
}

/* Mark the roots so they will be skipped by the GC,  */
static void lGCMark(){
	lRootsMark();
	lMarkFree();
}

void (*sweeperChain)() = NULL;
#define ALLOC_MAX MAX(lBytecodeArrayMax,MAX(lValMax,MAX(lClosureMax,MAX(lStringMax,MAX(lSymbolMax,MAX(lArrayMax,lTreeMax))))))
/* Free all values that have not been marked by lGCMark */
static void lGCSweep(){
	for(uint i=0;i<ALLOC_MAX;i++){
		if(i < lValMax)           {lValMarkMap[i]           ? lValMarkMap[i]           = 0 : lValFree(&lValList[i]);}
		if(i < lClosureMax)       {lClosureMarkMap[i]       ? lClosureMarkMap[i]       = 0 : lClosureFree(&lClosureList[i]);}
		if(i < lStringMax)        {lStringMarkMap[i]        ? lStringMarkMap[i]        = 0 : lStringFree(&lStringList[i]);}
		if(i < lSymbolMax)        {lSymbolMarkMap[i]        ? lSymbolMarkMap[i]        = 0 : lSymbolFree(&lSymbolList[i]);}
		if(i < lArrayMax)         {lArrayMarkMap[i]         ? lArrayMarkMap[i]         = 0 : lArrayFree(&lArrayList[i]);}
		if(i < lTreeMax)          {lTreeMarkMap[i]          ? lTreeMarkMap[i]          = 0 : lTreeFree(&lTreeList[i]);}
		if(i < lBytecodeArrayMax) {lBytecodeArrayMarkMap[i] ? lBytecodeArrayMarkMap[i] = 0 : lBytecodeArrayFree(&lBytecodeArrayList[i]);}
	}
	if(sweeperChain != NULL){sweeperChain();}
}

/* Force a garbage collection cycle, shouldn't need to be called manually since
 * when the heap is exhausted the GC is run */
void lGarbageCollect(){
	lGCRuns++;
	lGCMark();
	lGCSweep();
}
