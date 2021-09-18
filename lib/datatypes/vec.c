/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"
#include "vec.h"
#include "../nujel.h"

lVec     lVecList[VEC_MAX];
uint     lVecActive = 0;
uint     lVecMax    = 1;
uint     lVecFFree  = 0;

void lInitVec(){
	lVecActive  = 0;
	lVecMax     = 1;
}

void lVecFree(uint i){
	if((i == 0) || (i >= lVecMax)){return;}
	lVec *v = &lVecList[i];
	if(v->nextFree != 0){return;}
	lVecActive--;
	v->nextFree   = lVecFFree;
	v->flags      = 0;
	lVecFFree = i;
}

uint lVecAlloc(){
	lVec *ret;
	if(lVecFFree == 0){
		if(lVecMax >= VEC_MAX-1){
			lPrintError("lVec OOM ");
			return 0;
		}
		ret = &lVecList[lVecMax++];
	}else{
		ret = &lVecList[lVecFFree & VEC_MASK];
		lVecFFree = ret->nextFree;
	}
	lVecActive++;
	*ret = (lVec){0};
	return ret - lVecList;
}

lVal *lValVec(const vec v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltVec;
	ret->vCdr = lVecAlloc();
	if(ret->vCdr == 0){
		lValFree(ret);
		return NULL;
	}
	lVecV(ret->vCdr) = v;
	return ret;
}
