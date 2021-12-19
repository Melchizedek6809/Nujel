/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"

#include "../display.h"
#include "../allocation/garbage-collection.h"
#include "../collection/string.h"
#include "../type/closure.h"

lVal     lValList[VAL_MAX];
uint     lValActive = 0;
uint     lValMax    = 0;
lVal    *lValFFree  = NULL;

#include <string.h>
#include <stdlib.h>

/* Initialize the val allocator */
void lValInit(){
	lValActive = 0;
	lValMax    = 0;
}

/* Return a newly allocated value */
lVal *lValAlloc(){
	lVal *ret;
	if(lValFFree == NULL){
		if(lValMax >= VAL_MAX-1){
			lGarbageCollect();
			if(lValFFree == NULL){
				lPrintError("lVal OOM\n");
				exit(1);
			}else{
				ret       = lValFFree;
				lValFFree = ret->nextFree;
			}
		}else{
			ret = &lValList[lValMax++];
		}
	}else{
		ret       = lValFFree;
		lValFFree = ret->nextFree;
	}
	lValActive++;
	memset(ret, 0, sizeof(lVal));
	return ret;
}

/* Free the value V, should only be called by the GC */
void lValFree(lVal *v){
	if(v == NULL){return;}
	lValActive--;
	v->nextFree = lValFFree;
	lValFFree   = v;
}
