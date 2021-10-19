/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "array.h"

#include "../type-system.h"
#include "../allocator/garbage-collection.h"

#ifndef COSMOPOLITAN_H_
	#include <stdlib.h>
#endif

lArray   lArrayList[ARR_MAX];
uint     lArrayActive = 0;
uint     lArrayMax    = 1;
lArray  *lArrayFFree  = NULL;

void lInitArray(){
	lArrayActive = 0;
	lArrayMax    = 1;
}

lArray *lArrayAlloc(){
	lArray *ret;
	if(lArrayFFree == NULL){
		if(lArrayMax >= ARR_MAX-1){
			lGarbageCollect();
			if(lArrayFFree == NULL){
				lPrintError("lArray OOM ");
				return NULL;
			}else{
				ret = lArrayFFree;
				lArrayFFree = ret->nextFree;
			}
		}else{
			ret = &lArrayList[lArrayMax++];
		}
	}else{
		ret = lArrayFFree;
		lArrayFFree = ret->nextFree;
	}
	lArrayActive++;
	*ret = (lArray){0};
	return ret;
}

void lArrayFree(lArray *v){
	if(v == NULL){return;}
	lArrayActive--;
	if((v->data != NULL) && (v->length > 0)){
		free(v->data);
	}
	v->data     = NULL;
	v->nextFree = lArrayFFree;
	lArrayFFree = v;
}
