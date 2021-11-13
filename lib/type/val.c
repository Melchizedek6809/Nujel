/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"
#include <math.h>

#include "../exception.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/val.h"
#include "../collection/string.h"
#include "../type/closure.h"


lVal *lValInt(int v){
	lVal *ret = lValAlloc();
	ret->type = ltInt;
	ret->vInt = v;
	return ret;
}

lVal *lValFloat(float v){
	if(isnan(v)){
		lExceptionThrow(":float-nan","NaN is disallowed in Nujel, please check you calculations");
	}else if(isinf(v)){
		lExceptionThrow(":float-inf","INF is disallowed in Nujel, please check you calculations");
	}
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

lVal *lValObject(lClosure *v){
	lVal *ret = lValAlloc();
	ret->type = ltObject;
	ret->vClosure = v;
	return ret;
}
