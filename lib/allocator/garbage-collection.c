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
#include "../types/array.h"
#include "../types/closure.h"
#include "../types/list.h"
#include "../types/native-function.h"
#include "../types/string.h"
#include "../types/val.h"
#include "../types/vec.h"
#include "../nujel.h"

int lGCRuns = 0;

u8 lValMarkMap    [CLO_MAX];
u8 lClosureMarkMap[CLO_MAX];
u8 lArrayMarkMap  [ARR_MAX];
u8 lStringMarkMap [STR_MAX];

void lStringGCMark(lString *v){
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
		lValGCMark(lCar(v));
		lValGCMark(lCdr(v));
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
void lArrayGCMark(lArray *v){
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

/* Scan through the whole heap so we can mark the roots, terribly inefficient implementation! */
static void lGCMark(){
	lRootsClosureMark();
	lRootsValMark();
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

/* Force a garbage collection cycle, probably not what you want */
void lGarbageCollectForce(){
	lGCRuns++;
	lGCMark();
	lGCSweep();
}

/* Check if a garbage collction cycle is needed, and if so, do so */
void lGarbageCollect(){
	static int calls = 0;

	int thresh =         (VAL_MAX - (int)lValActive)     - (VAL_MAX / 128);
	thresh = MIN(thresh,((CLO_MAX - (int)lClosureActive) - 128) *  8);
	thresh = MIN(thresh,((ARR_MAX - (int)lArrayActive)   -  64) * 16);
	thresh = MIN(thresh,((STR_MAX - (int)lStringActive)  -  64) * 16);
	thresh = MIN(thresh,((VEC_MAX - (int)lVecActive)     -  64) * 16);
	if(++calls < thresh){return;}
	lGarbageCollectForce();
	calls = 0;
}
