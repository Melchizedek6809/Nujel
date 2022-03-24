/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "val.h"
#include "../display.h"

#include <string.h>
#include <stdlib.h>

lVal     lValList[VAL_MAX];
uint     lValActive = 0;
uint     lValMax    = 0;
lVal    *lValFFree  = NULL;

/* Initialize the val allocator */
void lValInit(){
	lValActive = 0;
	lValMax    = 0;
}

/* Return a newly allocated value */
lVal *lValAlloc(lType t){
	lVal *ret;
	if(lValFFree == NULL){
		if(lValMax >= VAL_MAX-1){
			lGarbageCollect();
			if(lValFFree == NULL){
				lPrintError("lVal OOM\n");
				exit(1);
			}else{
				goto allocateFromFreeList;
			}
		}else{
			ret = &lValList[lValMax++];
		}
	}else{
		allocateFromFreeList:
		ret       = lValFFree;
		lValFFree = ret->nextFree;
	}
	lValActive++;
	memset(ret, 0, sizeof(lVal));
	ret->type = t;
	return ret;
}

/* Free the value V, should only be called by the GC */
void lValFree(lVal *v){
	if(v->type == ltBytecodeArr){
		free((void *)v->vBytecodeArr.data);
		v->vBytecodeArr.data = NULL;
	}
	lValActive--;
	v->nextFree = lValFFree;
	lValFFree   = v;
}
