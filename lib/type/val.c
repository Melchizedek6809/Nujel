/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "val.h"
#include <math.h>

#include "../exception.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/val.h"
#include "../collection/string.h"
#include "../collection/tree.h"
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
		lExceptionThrow("float-nan","NaN is disallowed in Nujel, please check you calculations");
	}else if(isinf(v)){
		lExceptionThrow("float-inf","INF is disallowed in Nujel, please check you calculations");
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
	ret->vTree = v ? v : lTreeNew(NULL, NULL);
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

/* Return a newly allocated comment, something that should be ignored */
lVal *lValComment(){
	lVal *ret = lValAlloc();
	ret->type = ltComment;
	return ret;
}

int lValCompare(const lVal *a, const lVal *b){
	if((a == NULL) || (b == NULL)){
		return ((a == NULL) && (b == NULL)) ? 0 : 2;
	}
	if(a->type != b->type){return 2;}
	switch(a->type){
	case ltNoAlloc:
	default:
		return 2;
	case ltArray:
		return (a->vArray == b->vArray) ? 0 : -1;
	case ltTree:
		return (a->vTree == b->vTree) ? 0 : -1;
	case ltPair:
		if(b->type != ltPair){
			return -1;
		}else{
			return !((a->vList.car == b->vList.car)
				&& (a->vList.cdr == b->vList.cdr));
		}
	case ltKeyword:
	case ltSymbol:
		return (b->vSymbol != a->vSymbol) ? -1 : 0;
	case ltObject:
	case ltMacro:
	case ltLambda:
		return (b->vClosure != a->vClosure) ? -1 : 0;
	case ltNativeFunc:
	case ltSpecialForm:
		return (b->vNFunc != a->vNFunc) ? -1 : 0;
	case ltBytecodeOp:
		return a->vBytecodeOp != b->vBytecodeOp;
	case ltBool:
		return a->vBool != b->vBool;
        case ltGUIWidget:
		if(a->vInt == b->vInt){
			return  0;
		} else if(a->vInt < b->vInt) {
			return -1;
		} else {
			return  1;
		}
	case ltInt: {
		const i64 av = castToInt(a,0);
		const i64 bv = castToInt(b,0);
		if(bv == av){
			return  0;
		}else if(av < bv){
			return -1;
		}else{
			return  1;
		}}
	case ltFloat: {
		const float av = castToFloat(a,0.f);
		const float bv = castToFloat(b,0.f);
		if(bv == av){
			return  0;
		}else if(av < bv){
			return -1;
		}else{
			return  1;
		}}
	case ltString: {
		const uint alen = lStringLength(a->vString);
		const uint blen = lStringLength(b->vString);
		const char *ab = a->vString->buf;
		const char *bb = b->vString->buf;
		for(uint i=0;i<alen;i++){
			const u8 ac = ab[i];
			const u8 bc = bb[i];
			if(ac == bc){continue;}
			if(ac < bc){return -1;}
			return 1;
		}
		if(alen != blen){
			if(alen < blen){
				return -1;
			}
			return -1;
		}
		return 0; }
	}
}
