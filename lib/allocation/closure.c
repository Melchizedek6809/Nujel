/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "closure.h"
#include "garbage-collection.h"
#include "../nujel.h"
#include "../display.h"
#include "../collection/list.h"
#include "../collection/tree.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <ctype.h>
#include <string.h>

lClosure  lClosureList[CLO_MAX];
uint      lClosureActive = 0;
uint      lClosureMax    = 0;
lClosure *lClosureFFree  = NULL;

void lClosureInit(){
	lClosureActive  = 0;
	lClosureMax     = 0;
}

lClosure *lClosureAlloc(){
	lClosure *ret;
	if(lClosureFFree == NULL){
		if(lClosureMax >= CLO_MAX-1){
			lGarbageCollect();
			if(lClosureFFree == NULL){
				lPrintError("lClosure OOM ");
				return 0;
			}else{
				ret = lClosureFFree;
				lClosureFFree = ret->nextFree;
			}
		}else{
			ret = &lClosureList[lClosureMax++];
		}
	}else{
		ret = lClosureFFree;
		lClosureFFree = ret->nextFree;
	}
	lClosureActive++;
	*ret = (lClosure){0};
	return ret;
}

void lClosureFree(lClosure *clo){
	if(clo == NULL){return;}
	lClosureActive--;
	clo->nextFree = lClosureFFree;
	lClosureFFree = clo;
}

int lClosureID(const lClosure *n){
	return n - lClosureList;
}