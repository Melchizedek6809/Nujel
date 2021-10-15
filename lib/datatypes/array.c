/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "array.h"

#include "../casting.h"

#ifndef COSMOPOLITAN_H_
	#include <stdlib.h>
#endif

lArray   lArrayList[ARR_MAX];
uint     lArrayActive = 0;
uint     lArrayMax    = 1;
uint     lArrayFFree  = 0;

void lInitArray(){
	lArrayActive    = 0;
	lArrayMax       = 1;
}

u32 lArrayAlloc(){
	lArray *ret;
	if(lArrayFFree == 0){
		if(lArrayMax >= ARR_MAX-1){
			lPrintError("lArray OOM ");
			return 0;
		}
		ret = &lArrayList[lArrayMax++];
	}else{
		ret = &lArrayList[lArrayFFree & ARR_MASK];;
		lArrayFFree = ret->nextFree;
	}
	lArrayActive++;
	*ret = (lArray){0};
	return ret - lArrayList;
}

void lArrayFree(u32 v){
	v = v & ARR_MASK;
	if((v == 0) || (v >= lArrayMax)){return;}
	lArrayActive--;
	free(lArrayList[v].data);
	lArrayList[v].data = NULL;
	lArrayList[v].nextFree = lArrayFFree;
	lArrayFFree = v;
}
