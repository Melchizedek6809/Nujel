/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "garbage-collection.h"
#include "../types/array.h"
#include "../types/closure.h"
#include "../types/list.h"
#include "../types/native-function.h"
#include "../types/string.h"
#include "../types/val.h"
#include "../types/vec.h"
#include "../nujel.h"

int lGCRuns = 0;

static void lClosureGCMark(lClosure *c);
static void lValGCMark(lVal *v);
static void lArrayGCMark(lArray *v);
static void lNFuncGCMark(lNFunc *f);

static void lValGCMark(lVal *v){
	if((v == NULL) || (v->flags & lfMarked)){return;} // Circular refs
	v->flags |= lfMarked;

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
		if(v->vString == NULL){break;}
		v->vString->flags |= lfMarked;
		break;
	case ltVec:
		v->vVec->flags |= lfMarked;
		break;
	case ltSpecialForm:
	case ltNativeFunc:
		lNFuncGCMark(v->vNFunc);
		break;
	default:
		break;
	}
}

static void lClosureGCMark(lClosure *c){
	if((c == NULL) || (c->flags & lfMarked) || (!(c->flags & lfUsed))){return;} // Circular refs
	c->flags |= lfMarked;

	lValGCMark(c->data);
	lValGCMark(c->text);
	lClosureGCMark(c->parent);
}

static void lArrayGCMark(lArray *v){
	if(v == NULL){return;}
	v->flags |= lfMarked;
	for(int i=0;i<v->length;i++){
		if(v->data[i] == 0){continue;}
		lValGCMark(v->data[i]);
	}
}

static void lNFuncGCMark(lNFunc *f){
	if((f == NULL) || (f->flags & lfMarked)){return;}
	f->flags |= lfMarked;
	lValGCMark(f->doc);
}

static void lGCMark(){
	for(uint i=0;i<lValMax;i++){
		if(!(lValList[i].flags & lfNoGC)){continue;}
		lValGCMark(&lValList[i]);
	}
	for(uint i=0;i<lClosureMax;i++){
		if(!(lClosureList[i].flags & lfNoGC)){continue;}
		lClosureGCMark(&lClosureList[i]);
	}
	for(uint i=0;i<lStringMax;i++){
		if(!(lStringList[i].flags & lfNoGC)){continue;}
		lStringList[i].flags |= lfMarked;
	}
	for(uint i=0;i<lArrayMax;i++){
		if(!(lArrayList[i].flags & lfNoGC)){continue;}
		lArrayGCMark(&lArrayList[i]);
	}
	for(uint i=0;i<lNFuncMax;i++){
		if(!(lNFuncList[i].flags & lfNoGC)){continue;}
		lNFuncGCMark(&lNFuncList[i]);
	}
	for(uint i=0;i<lVecMax;i++){
		if(!(lVecList[i].flags & lfNoGC)){continue;}
		lVecList[i].flags |= lfMarked;
	}
}

static void lGCSweep(){
	for(uint i=0;i<lValMax;i++){
		if(lValList[i].flags & lfMarked){
			lValList[i].flags &= ~lfMarked;
			continue;
		}
		lValFree(&lValList[i]);
	}
	for(uint i=0;i<lClosureMax;i++){
		if(lClosureList[i].flags & lfMarked){
			lClosureList[i].flags &= ~lfMarked;
			continue;
		}
		lClosureFree(&lClosureList[i]);
	}
	for(uint i=0;i<lStringMax;i++){
		if(lStringList[i].flags & lfMarked){
			lStringList[i].flags &= ~lfMarked;
			continue;
		}
		lStringFree(&lStringList[i]);
	}
	for(uint i=0;i<lArrayMax;i++){
		if(lArrayList[i].flags & lfMarked){
			lArrayList[i].flags &= ~lfMarked;
			continue;
		}
		lArrayFree(&lArrayList[i]);
	}
	for(uint i=0;i<lNFuncMax;i++){
		if(lNFuncList[i].flags & lfMarked){
			lNFuncList[i].flags &= ~lfMarked;
			continue;
		}
		lNFuncFree(i);
	}
	for(uint i=0;i<lVecMax;i++){
		if(lVecList[i].flags & lfMarked){
			lVecList[i].flags &= ~lfMarked;
			continue;
		}
		lVecFree(i);
	}
}

void lGarbageCollectForce(){
	lGCRuns++;
	lGCMark();
	lGCSweep();
}

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
