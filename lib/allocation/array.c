/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "array.h"

#include "../display.h"
#include "../type-system.h"
#include "../allocation/garbage-collection.h"

#include <stdlib.h>

lArray   lArrayList[ARR_MAX];
uint     lArrayActive = 0;
uint     lArrayMax    = 0;
lArray  *lArrayFFree  = NULL;

/* Initialize the array allocator */
void lArrayInit(){
	lArrayActive = 0;
	lArrayMax    = 0;
}

/* Return a newly allocated lArray */
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
	return ret;
}

/* Free the array V, should never be called outside of the GC! */
void lArrayFree(lArray *v){
	if(v == NULL){return;}
	lArrayActive--;
	free(v->data);
	v->data     = NULL;
	v->nextFree = lArrayFFree;
	lArrayFFree = v;
}
