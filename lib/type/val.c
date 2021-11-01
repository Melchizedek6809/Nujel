/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"

#include "../allocation/val.h"
#include "../collection/string.h"
#include "../collection/closure.h"
#include "../allocation/garbage-collection.h"

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

lVal *lValDup(const lVal *v){
	return v == NULL ? NULL : lValCopy(lValAlloc(),v);
}

lVal *lValInf(){
	lVal *ret = lValAlloc();
	ret->type = ltInf;
	return ret;
}

lVal *lValInt(int v){
	lVal *ret = lValAlloc();
	ret->type = ltInt;
	ret->vInt = v;
	return ret;
}

lVal *lValFloat(float v){
	lVal *ret   = lValAlloc();
	ret->type   = ltFloat;
	ret->vFloat = v;
	return ret;
}

lVal *lValVec(const vec v){
	lVal *ret = lValAlloc();
	ret->type = ltVec;
	ret->vVec = v;
	return ret;
}

lVal *lValBool(bool v){
	lVal *ret = lValAlloc();
	ret->type = ltBool;
	ret->vBool = v;
	return ret;
}

lVal *lValTree(lTree *v){
	lVal *ret = lValAlloc();
	ret->type = ltTree;
	ret->vTree = v;
	return ret;
}
