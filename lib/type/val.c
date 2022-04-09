/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
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
	lVal *ret = lValAlloc(ltInt);
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
	lVal *ret   = lValAlloc(ltFloat);
	ret->vFloat = v;
	return ret;
}

/* Return a newly allocated Nujel vec of value V */
lVal *lValVec(const vec v){
	lVal *ret = lValAlloc(ltVec);
	ret->vVec = v;
	return ret;
}

/* Return a newly allocated Nujel bool of value V */
lVal *lValBool(bool v){
	lVal *ret = lValAlloc(ltBool);
	ret->vBool = v;
	return ret;
}

/* Return a newly allocated Nujel tree of value V */
lVal *lValTree(lTree *v){
	lVal *ret = lValAlloc(ltTree);
	ret->vTree = v ? v : lTreeNew(NULL, NULL);
	return ret;
}

/* Return a newly allocated Nujel object of value V */
lVal *lValObject(lClosure *v){
	lVal *ret = lValAlloc(ltObject);
	ret->vClosure = v;
	return ret;
}

/* Return a newly allocated Nujel lambda of value V */
lVal *lValLambda(lClosure *v){
	lVal *ret = lValAlloc(ltLambda);
	ret->vClosure = v;
	return ret;
}

/* Compare two values, returns an integer with multiple meanings:
 *   1 -> a > b
 *   0 -> a == b
 *  -1 -> a < b
 *   2 -> a != b
 */
int lValCompare(const lVal *a, const lVal *b){
	if((a == NULL) || (b == NULL)){
		return ((a == NULL) && (b == NULL)) ? 0 : 2;
	}
	if(a->type != b->type){return 2;}
	switch(a->type){
	default:
		return 2;
	case ltArray:
		return (a->vArray == b->vArray) ? 0 : -1;
	case ltTree:
		return (a->vTree == b->vTree) ? 0 : -1;
	case ltPair:
		return !((a->vList.car == b->vList.car)
				&& (a->vList.cdr == b->vList.cdr));
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
		return (b->vPointer != a->vPointer) ? -1 : 0;
	case ltInt: {
		const i64 av = a->vInt;
		const i64 bv = b->vInt;
		if(bv == av){
			return  0;
		}else if(av < bv){
			return -1;
		}else{
			return  1;
		}}
	case ltFloat: {
		const float av = a->vFloat;
		const float bv = b->vFloat;
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
		const uint len = MIN(alen,blen);
		const char *ab = a->vString->buf;
		const char *bb = b->vString->buf;
		for(uint i=0;i<len;i++){
			const u8 ac = *ab++;
			const u8 bc = *bb++;
			if(ac != bc){
				return ac < bc ? -1 : 1;
			}
		}
		return alen == blen ? 0 : (alen < blen ? -1 : 1);}
	}
}
