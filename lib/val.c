/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>

lVal *lValInt(i64 v){
	lVal *ret = lValAlloc(ltInt);
	ret->vInt = v;
	return ret;
}

lVal *lValFloat(lClosure *c, double v){
	if(unlikely(isnan(v))){
		lExceptionThrowValClo("float-nan","NaN is disallowed in Nujel", NULL, c);
	}else if(unlikely(isinf(v))){
		lExceptionThrowValClo("float-inf","INF is disallowed in Nujel", NULL, c);
	}
	lVal *ret   = lValAlloc(ltFloat);
	ret->vFloat = v;
	return ret;
}

lVal *lValBool(bool v){
	lVal *ret = lValAlloc(ltBool);
	ret->vBool = v;
	return ret;
}

lVal *lValTree(lTree *v){
	lVal *ret = lValAlloc(ltTree);
	ret->vTree = v ? v : lTreeNew(NULL, NULL);
	return ret;
}

lVal *lValEnvironment(lClosure *v){
	lVal *ret = lValAlloc(ltEnvironment);
	ret->vClosure = v;
	return ret;
}

lVal *lValLambda(lClosure *v){
	lVal *ret = lValAlloc(ltLambda);
	ret->vClosure = v;
	return ret;
}

/* Checks if A is greater than B, returns 0 if the two values can't be compared
 | or if they are equal.
 */
i64 lValGreater(const lVal *a, const lVal *b){
	if(unlikely((a == NULL) || (b == NULL))){return 0;}
	if(unlikely(a->type != b->type)){
		if(likely(((a->type == ltInt) || (a->type == ltFloat)) && ((b->type == ltInt) || (b->type == ltFloat)))){
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
		const char *ab = a->vString->data;
		const char *bb = b->vString->data;
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
	if(unlikely((a == NULL) || (b == NULL))){
		return ((a == NULL) && (b == NULL));
	}
	if(unlikely(a->type != b->type)){
		if(likely(((a->type == ltInt) || (a->type == ltFloat)) && ((b->type == ltInt) || (b->type == ltFloat)))){
			return ((a->type == ltInt) ? (float)a->vInt : a->vFloat) == ((b->type == ltInt) ? (float)b->vInt : b->vFloat);
		}else{
			return false;
		}
	}
	switch(a->type){
	default:
		return false;
	case ltPair:
		return (a->vList->car == b->vList->car) && (a->vList->cdr == b->vList->cdr);
	case ltArray:
		return a->vArray == b->vArray;
	case ltTree:
		return a->vTree == b->vTree;
	case ltKeyword:
	case ltSymbol:
		return b->vSymbol == a->vSymbol;
	case ltEnvironment:
	case ltMacro:
	case ltLambda:
		return b->vClosure == a->vClosure;
	case ltNativeFunc:
		return b->vNFunc == a->vNFunc;
	case ltBytecodeOp:
		return a->vBytecodeOp == b->vBytecodeOp;
	case ltBuffer:
		return a->vBuffer == b->vBuffer;
	case ltBufferView:
		return a->vBufferView == b->vBufferView;
	case ltBool:
		return a->vBool == b->vBool;
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

/* Return a newly allocated nujel symbol of value S */
lVal *lValSymS(const lSymbol *s){
	if(unlikely(s == NULL)){return NULL;}
	lVal *ret = lValAlloc(ltSymbol);
	ret->vSymbol = s;
	return ret;
}

/* Return a nujel value for the symbol within S */
lVal *lValSym(const char *s){
	return lValSymS(lSymS(s));
}

/* Return a newly allocated nujel keyword of value S */
lVal *lValKeywordS(const lSymbol *s){
	if(unlikely(s == NULL)){return NULL;}
	lVal *ret = lValAlloc(ltKeyword);
	ret->vSymbol = s;
	return ret;
}

/* Return a nujel value for the keyword within S */
lVal *lValKeyword(const char *s){
	return lValKeywordS(lSymS(s));
}

lVal *lValFileHandle(FILE *fh){
	lVal *ret = lValAlloc(ltFileHandle);
	ret->vFileHandle = fh;
	return ret;
}

lVal *lValBytecodeOp(lBytecodeOp v){
	lVal *ret = lValAlloc(ltBytecodeOp);
	ret->vBytecodeOp = v;
	return ret;
}
