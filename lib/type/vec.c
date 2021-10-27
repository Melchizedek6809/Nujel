/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"
#include "vec.h"
#include "../nujel.h"

#include <string.h>

lVec     lVecList[VEC_MAX];
uint     lVecActive = 0;
uint     lVecMax    = 0;
lVec    *lVecFFree  = NULL;

void lInitVec(){
	lVecActive  = 0;
	lVecMax     = 0;
}

void lVecFree(lVec *v){
	if(v == NULL){return;}
	lVecActive--;
	v->nextFree   = lVecFFree;
	lVecFFree     = v;
}

lVec *lVecAlloc(){
	lVec *ret;
	if(lVecFFree == NULL){
		if(lVecMax >= VEC_MAX-1){
			lPrintError("lVec OOM ");
			return 0;
		}
		ret = &lVecList[lVecMax++];
	}else{
		ret = lVecFFree;
		lVecFFree = ret->nextFree;
	}
	lVecActive++;
	memset(ret,0,sizeof(lVec));
	return ret;
}

lVal *lValVec(const vec v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltVec;
	ret->vVec = lVecAlloc();
	if(ret->vVec == NULL){
		return NULL;
	}else{
		ret->vVec->v = v;
		return ret;
	}
}
