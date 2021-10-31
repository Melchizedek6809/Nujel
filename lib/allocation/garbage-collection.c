/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 *
 * Contains a terrible implementation of a mark-sweep garbage collector, but it
 * is good enough for now.
 */
#include "array.h"
#include "closure.h"
#include "garbage-collection.h"
#include "roots.h"
#include "string.h"
#include "tree.h"
#include "val.h"
#include "../nujel.h"
#include "../operation/time.h"
#include "../collection/closure.h"
#include "../collection/list.h"
#include "../collection/string.h"
#include "../collection/tree.h"
#include "../type/native-function.h"

#include <stdio.h>

int lGCRuns = 0;

u8 lValMarkMap    [VAL_MAX];
u8 lTreeMarkMap   [TRE_MAX];
u8 lClosureMarkMap[CLO_MAX];
u8 lArrayMarkMap  [ARR_MAX];
u8 lStringMarkMap [STR_MAX];

/* Mark v as being in use so it won't get freed by the GC */
void lStringGCMark(const lString *v){
	if(v == NULL){return;}
	const uint ci = v - lStringList;
	if(lStringMarkMap[ci]){return;}
	lStringMarkMap[ci] = 1;
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lValGCMark(lVal *v){
	if(v == NULL){return;}
	const uint ci = v - lValList;
	if(lValMarkMap[ci]){return;}
	lValMarkMap[ci] = 1;

	switch(v->type){
	case ltPair:
		lValGCMark(v->vList.car);
		lValGCMark(v->vList.cdr);
		break;
	case ltObject:
	case ltDynamic:
	case ltLambda:
		lClosureGCMark(v->vClosure);
		break;
	case ltArray:
		lArrayGCMark(v->vArray);
		break;
	case ltSpecialForm:
	case ltNativeFunc:
		lValGCMark(v->vNFunc->doc);
		break;
	case ltString:
		lStringGCMark(v->vString);
		break;
	case ltTree:
		lTreeGCMark(v->vTree);
		break;
	default:
		break;
	}
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lTreeGCMark(const lTree *v){
	if(v == NULL){return;}
	const uint ci = v - lTreeList;
	if(lTreeMarkMap[ci]){return;}
	lTreeMarkMap[ci] = 1;
	lTreeGCMark(v->left);
	lTreeGCMark(v->right);
	lValGCMark(v->value);
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lClosureGCMark(const lClosure *c){
	if(c == NULL){return;}
	const uint ci = c - lClosureList;

	if(lClosureMarkMap[ci]){return;}
	lClosureMarkMap[ci] = 1;

	lTreeGCMark(c->data);
	lValGCMark(c->doc);
	lValGCMark(c->text);
	lValGCMark(c->args);
	lClosureGCMark(c->parent);
}

/* Mark v and all refeferences within as being in use so it won't get freed when sweeping */
void lArrayGCMark(const lArray *v){
	if(v == NULL){return;}
	const uint ci = v - lArrayList;
	if(lArrayMarkMap[ci]){return;}
	lArrayMarkMap[ci] = 1;
	for(int i=0;i<v->length;i++){
		lValGCMark(v->data[i]);
	}
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
}

/* Mark the roots so they will be skipped by the GC,  */
static void lGCMark(){
	lRootsMark();
	lMarkFree();
}

/* Free all values that have not been marked by lGCMark */
static void lGCSweep(){
	for(uint i=0;i<lValMax;i++){
		if(lValMarkMap[i]){
			lValMarkMap[i] = 0;
		}else{
			lValFree(&lValList[i]);
		}
	}
	for(uint i=0;i<lClosureMax;i++){
		if(lClosureMarkMap[i]){
			lClosureMarkMap[i] = 0;
		}else{
			lClosureFree(&lClosureList[i]);
		}
	}
	for(uint i=0;i<lStringMax;i++){
		if(lStringMarkMap[i]){
			lStringMarkMap[i] = 0;
		}else{
			lStringFree(&lStringList[i]);
		}
	}
	for(uint i=0;i<lArrayMax;i++){
		if(lArrayMarkMap[i]){
			lArrayMarkMap[i] = 0;
		}else{
			lArrayFree(&lArrayList[i]);
		}
	}
	for(uint i=0;i<lTreeMax;i++){
		if(lTreeMarkMap[i]){
			lTreeMarkMap[i] = 0;
		}else{
			lTreeFree(&lTreeList[i]);
		}
	}
}

/* Force a garbage collection cycle, shouldn't need to be called manually since
 * when the heap is exhausted the GC is run */
void lGarbageCollect(){
	const int bva = lValActive;
	const int bca = lClosureActive;
	const int baa = lArrayActive;
	const int bsa = lStringActive;
	const int bta = lTreeActive;
	const u64 start = getMSecs();

	lGCRuns++;
	lGCMark();
	lGCSweep();
	if(lVerbose){
		const u64 end = getMSecs();
		printf("== Garbage Collection #%u took %ims ==\n",lGCRuns,(int)(end-start));
		printf("Vals: %u -> %u [Δ %i]{Σ %i}\n",bva,lValActive,(int)lValActive - bva, lValMax);
		printf("Clos: %u -> %u [Δ %i]{Σ %i}\n",bca,lClosureActive,(int)lClosureActive - bca, lClosureMax);
		printf("Strs: %u -> %u [Δ %i]{Σ %i}\n",bsa,lStringActive,(int)lStringActive - bsa, lStringMax);
		printf("Arrs: %u -> %u [Δ %i]{Σ %i}\n",baa,lArrayActive, (int)lArrayActive - baa, lArrayMax);
		printf("Tres: %u -> %u [Δ %i]{Σ %i}\n",bta,lTreeActive, (int)lTreeActive - bta, lTreeMax);
		printf("--------------\n\n");
	}
}
