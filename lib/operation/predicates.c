/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "predicates.h"

#include "arithmetic.h"
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
		return !((a == NULL) && (b == NULL));
	}
	lType ct = lTypecast(a->type, b->type);
	switch(ct){
	case ltPair:
		if(b->type != ltPair){
			return -1;
		}else{
			return !((a->vList.car == b->vList.car)
				&& (a->vList.cdr == b->vList.cdr));
		}
	case ltNoAlloc:
	default:
		return 2;
	case ltSymbol:
	case ltLambda:
	case ltNativeFunc:
	case ltSpecialForm:
		if(a->type   != b->type)  {return -1;}
		if(b->vNFunc != a->vNFunc){return -1;}
		return 0;
	case ltInf:
		if((a == NULL) || (b == NULL)){return  2;}
		if((a->type == ltInf) && (b->type == ltInf)){return 0;}
		if(a->type == ltInf){return 1;}
		return -1;
	case ltBool:
		if((b->type != ltBool) || (a->type != ltBool)){
			return -1;
		}else{
			return castToBool(a) != castToBool(b);
		}
	case ltInt: {
		if((a == NULL) || (b == NULL)){return  2;}
		const int av = castToInt(a,0);
		const int bv = castToInt(b,0);
		if(bv == av){
			return  0;
		}else if(av < bv){
			return -1;
		}else{
			return  1;
		}}
	case ltFloat: {
		if((a == NULL) || (b == NULL)) {return  2;}
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

void lOperationsPredicate(lClosure *c){
	lAddInfix(lAddNativeFunc(c,"less? <",           "[a b]","#t if A < B",  lnfLess));
	lAddInfix(lAddNativeFunc(c,"less-equal? <=",    "[a b]","#t if A <= B", lnfLessEqual));
	lAddInfix(lAddNativeFunc(c,"equal? eq? == =",   "[a b]","#t if A == B", lnfEqual));
	lAddInfix(lAddNativeFunc(c,"!= <>",             "[a b]","#t if A != B", lnfUnequal));
	lAddInfix(lAddNativeFunc(c,"greater-equal? >=", "[a b]","#t if A >= B", lnfGreaterEqual));
	lAddInfix(lAddNativeFunc(c,"greater? >",        "[a b]","#t if A > B",  lnfGreater));
	lAddNativeFunc(c,"nil?",                        "[a]",  "#t if A #nil", lnfNilPred);
}
