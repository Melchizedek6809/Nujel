/*
 * This file is a part of Nujel, licensed under the MIT License.
 */
#include "tree.h"

#include "garbage-collection.h"
#include "roots.h"
#include "../display.h"
#include "../collection/list.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <stdlib.h>

lTree    lTreeList[TRE_MAX];
uint     lTreeActive = 0;
uint     lTreeMax    = 0;
lTree   *lTreeFFree  = NULL;

void lTreeInit(){
	lTreeActive = 0;
	lTreeMax    = 0;
}

lTree *lTreeAlloc(){
	lTree *ret;
	if(lTreeFFree == NULL){
		if(lTreeMax >= TRE_MAX-1){
			lGarbageCollect();
			if(lTreeFFree == NULL){
				lPrintError("lTree OOM\n");
				exit(1);
			}else{
				ret       = lTreeFFree;
				lTreeFFree = ret->nextFree;
			}
		}else{
			ret = &lTreeList[lTreeMax++];
		}
	}else{
		ret       = lTreeFFree;
		lTreeFFree = ret->nextFree;
	}
	lTreeActive++;
	return ret;
}

void lTreeFree(lTree *t){
	if(t == NULL){return;}
	lTreeActive--;
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
}
