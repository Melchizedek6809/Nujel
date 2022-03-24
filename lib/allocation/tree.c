/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "tree.h"
#include "../display.h"

#include <stdlib.h>

lTree    lTreeList[TRE_MAX];
uint     lTreeActive = 0;
uint     lTreeMax    = 0;
lTree   *lTreeFFree  = NULL;

/* Initialize the tree allocator */
void lTreeInit(){
	lTreeActive = 0;
	lTreeMax    = 0;
}

/* Return a freshly allocated tree */
lTree *lTreeAlloc(){
	lTree *ret;
	if(lTreeFFree == NULL){
		if(lTreeMax >= TRE_MAX-1){
			lGarbageCollect();
			if(lTreeFFree == NULL){
				lPrintError("lTree OOM\n");
				exit(123);
			}else{
				goto allocateFromFreeList;
			}
		}else{
			ret = &lTreeList[lTreeMax++];
		}
	}else{
		allocateFromFreeList:
		ret        = lTreeFFree;
		lTreeFFree = ret->nextFree;
	}
	lTreeActive++;
	return ret;
}

/* Free the Tree T, should never be called outside of the GC */
void lTreeFree(lTree *t){
	if(t == NULL){return;}
	lTreeActive--;
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
}
