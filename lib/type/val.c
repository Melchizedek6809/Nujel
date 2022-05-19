/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "val.h"

#include "../allocation/garbage-collection.h"
#include "../allocation/allocator.h"
#include "../collection/tree.h"
#include "../collection/string.h"
#include "../type/closure.h"

#include <math.h>
#include <string.h>

/* Return a newly allocated Nujel int of value V */
lVal *lValInt(i64 v){
	lVal *ret = lValAlloc(ltInt);
	ret->vInt = v;
	return ret;
}

/* Return a newly allocated Nujel float of value V */
lVal *lValFloat(double v){
	if(isnan(v)){
		lExceptionThrowValClo("float-nan","NaN is disallowed in Nujel, please check you calculations", NULL, NULL);
	}else if(isinf(v)){
		lExceptionThrowValClo("float-inf","INF is disallowed in Nujel, please check you calculations", NULL, NULL);
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

/* Checks if A is greater than B, returns 0 if the two values can't be compared
 | or if they are equal.
 */
i64 lValGreater(const lVal *a, const lVal *b){
	if((a == NULL) || (b == NULL)){return 0;}
	if(a->type != b->type){
		if(((a->type == ltInt) || (a->type == ltFloat)) && ((b->type == ltInt) || (b->type == ltFloat))){
			return ((a->type == ltInt) ? (float)a->vInt : a->vFloat) < ((b->type == ltInt) ? (float)b->vInt : b->vFloat) ? -1 : 1;
		}else{
			return 0;
		}
	}
	switch(a->type){
	default:
		return 0;
	case ltKeyword:
	case ltSymbol: {
		const uint alen = strnlen(a->vSymbol->c, sizeof(a->vSymbol->c));
		const uint blen = strnlen(b->vSymbol->c, sizeof(b->vSymbol->c));
		const uint len = MIN(alen,blen);
		const char *ab = a->vSymbol->c;
		const char *bb = b->vSymbol->c;
		for(uint i=0;i<len;i++){
			const u8 ac = *ab++;
			const u8 bc = *bb++;
			if(ac != bc){
				return ac - bc;
			}
		}
		return alen - blen;
	}

	case ltInt:
		return a->vInt - b->vInt;
	case ltFloat:
		return a->vFloat < b->vFloat ? -1 : 1;
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
				return ac - bc;
			}
		}
		return alen - blen;
	}}
}

/* Check two values for equality */
bool lValEqual(const lVal *a, const lVal *b){
	if((a == NULL) || (b == NULL)){
		return ((a == NULL) && (b == NULL));
	}
	if(a->type != b->type){
		if(((a->type == ltInt) || (a->type == ltFloat)) && ((b->type == ltInt) || (b->type == ltFloat))){
			return ((a->type == ltInt) ? (float)a->vInt : a->vFloat) == ((b->type == ltInt) ? (float)b->vInt : b->vFloat);
		}else{
			return false;
		}
	}
	switch(a->type){
	default:
		return false;
	case ltPair:
		return (a->vList.car == b->vList.car) && (a->vList.cdr == b->vList.cdr);
	case ltArray:
		return a->vArray == b->vArray;
	case ltTree:
		return a->vTree == b->vTree;
	case ltKeyword:
	case ltSymbol:
		return b->vSymbol == a->vSymbol;
	case ltObject:
	case ltMacro:
	case ltLambda:
		return b->vClosure == a->vClosure;
	case ltNativeFunc:
		return b->vNFunc == a->vNFunc;
	case ltBytecodeOp:
		return a->vBytecodeOp == b->vBytecodeOp;
	case ltBool:
		return a->vBool == b->vBool;
        case ltGUIWidget:
		return b->vPointer == a->vPointer;
	case ltInt:
		return a->vInt == b->vInt;
	case ltFloat:
		return a->vFloat == b->vFloat;
	case ltString: {
		const uint alen = lStringLength(a->vString);
		const uint blen = lStringLength(b->vString);
		return (alen == blen) && (strncmp(a->vString->data, b->vString->data, alen) == 0);
	}}
}
