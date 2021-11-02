/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"

#include "../display.h"
#include "../collection/string.h"
#include "../collection/closure.h"
#include "../allocation/garbage-collection.h"

lVal     lValList[VAL_MAX];
uint     lValActive = 0;
uint     lValMax    = 0;
lVal    *lValFFree  = NULL;

#include <stdlib.h>

void lValInit(){
	lValActive = 0;
	lValMax    = 0;
}

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
	*ret = (lVal){0};
	return ret;
}

void lGUIWidgetFree(lVal *v);
void lValFree(lVal *v){
	if(v == NULL){return;}
	lValActive--;
	v->nextFree = lValFFree;
	lValFFree   = v;
}