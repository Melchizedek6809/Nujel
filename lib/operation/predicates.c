/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"

#include "../nujel.h"
#include "../type-system.h"
#include "../collection/list.h"
#include "../collection/string.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>

static int lValCompare(lVal *v){
	if(v == NULL){return 2;}
	lVal *a = lCar(v);
	v = lCdr(v);
	if(v == NULL){return 2;}
	lVal *b = lCar(v);
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

static lVal *lnfLess(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(v);
	return lValBool(cmp == 2 ? false : cmp < 0);
}

static lVal *lnfUnequal(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(v);
	return lValBool(!(cmp == 2 ? false : cmp == 0));
}

static lVal *lnfEqual(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(v);
	return lValBool(cmp == 2 ? false : cmp == 0);
}

static lVal *lnfLessEqual(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(v);
	return lValBool(cmp == 2 ? false : cmp <= 0);
}

static lVal *lnfGreater(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(v);
	return lValBool(cmp == 2 ? false : cmp > 0);
}

static lVal *lnfGreaterEqual(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(v);
	return lValBool(cmp == 2 ? false : cmp >= 0);
}

static lVal *lnfNilPred(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lCar(v) == NULL);
}

static lVal *lnfKeywordPred(lClosure *c, lVal *v){
	(void)c;
	const lSymbol *s = castToSymbol(lCar(v), NULL);
	return lValBool(s ? lSymKeyword(s) : false);
}

void lOperationsPredicate(lClosure *c){
	lAddNativeFunc(c,"<",    "[α β]","Return true if α is less than β",  lnfLess);
	lAddNativeFunc(c,"<=",   "[α β]","Return true if α is less or equal to β", lnfLessEqual);
	lAddNativeFunc(c,"==",   "[α β]","Return true if α is equal to β", lnfEqual);
	lAddNativeFunc(c,"!=",   "[α β]","Return true if α is not equal to  β", lnfUnequal);
	lAddNativeFunc(c,">=",   "[α β]","Return true if α is greater or equal than β", lnfGreaterEqual);
	lAddNativeFunc(c,">",    "[α β]","Return true if α is greater than β",  lnfGreater);
	lAddNativeFunc(c,"nil?", "[α]",  "Return true if α is #nil", lnfNilPred);
	lAddNativeFunc(c,"keyword?", "[α]",  "Return true if α is a keyword symbol", lnfKeywordPred);
}
