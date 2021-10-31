/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"

#include "../collection/string.h"
#include "../collection/closure.h"
#include "../allocation/garbage-collection.h"

#include <stdlib.h>

lVal     lValList[VAL_MAX];
uint     lValActive = 0;
uint     lValMax    = 0;
lVal    *lValFFree  = NULL;

void lInitVal(){
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

lVal *lValCopy(lVal *dst, const lVal *src){
	if((dst == NULL) || (src == NULL)){return NULL;}
	*dst = *src;
	if(dst->type == ltString){
		dst->vString = lStringNew(src->vString->buf,lStringLength(src->vString));
	}else if(dst->type == ltPair){
		dst->vList.car = lValDup(dst->vList.car);
		dst->vList.cdr = lValDup(dst->vList.cdr);
	}
	return dst;
}

lVal *lValInf(){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltInf;
	return ret;
}

lVal *lValInt(int v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltInt;
	ret->vInt = v;
	return ret;
}

lVal *lValFloat(float v){
	lVal *ret   = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type   = ltFloat;
	ret->vFloat = v;
	return ret;
}

lVal *lValVec(const vec v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltVec;
	ret->vVec = v;
	return ret;
}

lVal *lValBool(bool v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltBool;
	ret->vBool = v;
	return ret;
}

lVal *lValTree(lTree *v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltTree;
	ret->vTree = v;
	return ret;
}

lVal *lValDup(const lVal *v){
	return v == NULL ? NULL : lValCopy(lValAlloc(),v);
}