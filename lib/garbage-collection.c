/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "garbage-collection.h"
#include "datatypes/array.h"
#include "datatypes/closure.h"
#include "datatypes/native-function.h"
#include "datatypes/string.h"
#include "datatypes/vec.h"
#include "nujel.h"

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
		lClosureGCMark(&lClo(v->vCdr));
		break;
	case ltArray:
		lArrayGCMark(&lArr(v));
		break;
	case ltString:
		lStrFlags(v) |= lfMarked;
		break;
	case ltVec:
		lVecFlags(v->vCdr) |= lfMarked;
		break;
	case ltNativeFunc:
		lNFuncGCMark(&lNFN(v->vCdr));
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
	lClosureGCMark(&lClo(c->parent));
}

static void lArrayGCMark(lArray *v){
	if((v == NULL) || (v->nextFree != 0)){return;}
	v->flags |= lfMarked;
	for(int i=0;i<v->length;i++){
		if(v->data[i] == 0){continue;}
		lValGCMark(lValD(v->data[i]));
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
		lVecFlags(i) |= lfMarked;
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
		lClosureFree(i);
	}
	for(uint i=0;i<lStringMax;i++){
		if(lStringList[i].flags & lfMarked){
			lStringList[i].flags &= ~lfMarked;
			continue;
		}
		lStringFree(i);
	}
	for(uint i=0;i<lArrayMax;i++){
		if(lArrayList[i].flags & lfMarked){
			lArrayList[i].flags &= ~lfMarked;
			continue;
		}
		lArrayFree(i);
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

static void lClosureDoGC(){
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
	lClosureDoGC();
	calls = 0;
}
