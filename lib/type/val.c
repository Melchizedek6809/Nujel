/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"
#include <math.h>

#include "../exception.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/val.h"
#include "../collection/string.h"
#include "../type/closure.h"

/* Return a newly allocated Nujel int of value V */
lVal *lValInt(i64 v){
	lVal *ret = lValAlloc();
	ret->type = ltInt;
	ret->vInt = v;
	return ret;
}

/* Return a newly allocated Nujel float of value V */
lVal *lValFloat(double v){
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

/* Return a newly allocated Nujel vec of value V */
lVal *lValVec(const vec v){
	lVal *ret = lValAlloc();
	ret->type = ltVec;
	ret->vVec = v;
	return ret;
}

/* Return a newly allocated Nujel bool of value V */
lVal *lValBool(bool v){
	lVal *ret = lValAlloc();
	ret->type = ltBool;
	ret->vBool = v;
	return ret;
}

/* Return a newly allocated Nujel tree of value V */
lVal *lValTree(lTree *v){
	lVal *ret = lValAlloc();
	ret->type = ltTree;
	ret->vTree = v;
	return ret;
}

/* Return a newly allocated Nujel object of value V */
lVal *lValObject(lClosure *v){
	lVal *ret = lValAlloc();
	ret->type = ltObject;
	ret->vClosure = v;
	return ret;
}

/* Return a newly allocated Nujel lambda of value V */
lVal *lValLambda(lClosure *v){
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vClosure = v;
	return ret;
}
