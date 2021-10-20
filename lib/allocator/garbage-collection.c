/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 *
 * Contains a terrible implementation of a mark-sweep garbage collector, but it
 * is good enough for now.
 */
#include "garbage-collection.h"
#include "roots.h"
#include "../operator/time.h"
#include "../types/array.h"
#include "../types/closure.h"
#include "../types/list.h"
#include "../types/native-function.h"
#include "../types/string.h"
#include "../types/val.h"
#include "../types/vec.h"
#include "../nujel.h"

#ifndef COSMOPOLITAN_H_
	#include <stdio.h>
#endif

int lGCRuns = 0;
bool lGCVerbose = false;

u8 lValMarkMap    [VAL_MAX];
u8 lClosureMarkMap[CLO_MAX];
u8 lArrayMarkMap  [ARR_MAX];
u8 lStringMarkMap [STR_MAX];

void lStringGCMark(const lString *v){
	if(v == NULL){return;}
	const uint ci = v - lStringList;
	if(lStringMarkMap[ci]){return;}
	lStringMarkMap[ci] = 1;
}

/* Mark v as being in use so it won't get freed when sweeping */
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
	case ltLambda:
		lClosureGCMark(v->vClosure);
		break;
	case ltArray:
		lArrayGCMark(v->vArray);
		break;
	case ltString:
		lStringGCMark(v->vString);
		break;
	default:
		break;
	}
}

/* Mark every reference for the GC to ignore contained in c */
void lClosureGCMark(const lClosure *c){
	if(c == NULL){return;}
	const uint ci = c - lClosureList;
	if(lClosureMarkMap[ci]){return;}
	lClosureMarkMap[ci] = 1;

	lValGCMark(c->data);
	lValGCMark(c->doc);
	lValGCMark(c->text);
	lClosureGCMark(c->parent);
}

/* Mark every reference for the GC to ignore contained in v */
void lArrayGCMark(const lArray *v){
	if(v == NULL){return;}
	const uint ci = v - lArrayList;
	if(lArrayMarkMap[ci]){return;}
	lArrayMarkMap[ci] = 1;
	for(int i=0;i<v->length;i++){
		if(v->data[i]){
			lValGCMark(v->data[i]);
		}
	}
}

static void lMarkFree(){
	for(lArray *arr = lArrayFFree;arr != NULL;arr = arr->nextFree){
		lArrayGCMark(arr);
	}
	for(lClosure *clo = lClosureFFree;clo != NULL;clo = clo->nextFree){
		lClosureGCMark(clo);
	}
	for(lString *str = lStringFFree;str != NULL;str = str->nextFree){
		lStringGCMark(str);
	}
	for(lVal *v = lValFFree;v != NULL;v = v->nextFree){
		lValGCMark(v);
	}
}

/* Scan through the whole heap so we can mark the roots, terribly inefficient implementation! */
static void lGCMark(){
	lRootsClosureMark();
	lRootsValMark();
	lRootsStringMark();
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
}

/* Force a garbage collection cycle, shouldn't need to be called manually since
 * when the heap is exhausted the GC is run */
void lGarbageCollect(){
	const int bva = lValActive;
	const int bca = lClosureActive;
	const int baa = lArrayActive;
	const int bsa = lStringActive;
	const u64 start = getMSecs();

	lGCRuns++;
	lGCMark();
	lGCSweep();
	if(lGCVerbose){
		const u64 end = getMSecs();
		printf("== Garbage Collection #%u took %lums ==\n",lGCRuns,end-start);
		printf("Vals: %u -> %u [%i] {%i}\n",bva,lValActive,(int)lValActive - bva, rootsValSP);
		printf("Clos: %u -> %u [%i] {%i}\n",bca,lClosureActive,(int)lClosureActive - bca, rootsClosureSP);
		printf("Arrs: %u -> %u [%i] {%i}\n",baa,lArrayActive,(int)lArrayActive - baa, 0);
		printf("Strs: %u -> %u [%i] {%i}\n",bsa,lStringActive,(int)lStringActive - bsa, rootsStringSP);
		printf("--------------\n\n");
	}
}
