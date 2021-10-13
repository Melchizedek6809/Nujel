/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "predicates.h"

#include "../casting.h"
#include "../nujel.h"
#include "../datatypes/list.h"
#include "../datatypes/native-function.h"
#include "../datatypes/string.h"
#include "../datatypes/val.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>

static int lValCompare(lClosure *c, lVal *v){
	if((v == NULL) || (lCar(v) == NULL) || (lCdr(v) == NULL)){return 2;}
	lVal *a = lCar(v);
	v = lCdr(v);
	if(lCar(v) == NULL){return 2;}
	lVal *b = lCar(v);
	if((a == NULL) || (b == NULL)){return 2;}
	lType ct = lTypecast(a->type, b->type);
	switch(ct){
	default:
		return 2;
	case ltSymbol:
	case ltLambda:
	case ltNativeFunc:
		if(a->type != b->type){return -1;}
		if(b->vCdr != a->vCdr){return -1;}
		return 0;
	case ltInf:
		if((a == NULL) || (b == NULL)){return  2;}
		if((a->type == ltInf) && (b->type == ltInf)){return 0;}
		if(a->type == ltInf){return 1;}
		return -1;
	case ltBool:
	case ltInt:
		a = lnfInt(c,a);
		b = lnfInt(c,b);
		if((a == NULL) || (b == NULL)){return  2;}
		if(b->vInt == a->vInt)        {return  0;}
		else if(a->vInt  < b->vInt)   {return -1;}
		return 1;
	case ltFloat:
		a = lnfFloat(c,a);
		b = lnfFloat(c,b);
		if((a == NULL) || (b == NULL)) {return  2;}
		if(b->vFloat == a->vFloat)     {return  0;}
		else if(a->vFloat  < b->vFloat){return -1;}
		return 1;
	case ltString: {
		const uint alen = lStringLength(&lStr(a));
		const uint blen = lStringLength(&lStr(b));
		const char *ab = lStrBuf(a);
		const char *bb = lStrBuf(b);
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
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp < 0);
}

static lVal *lnfEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp == 0);
}

static lVal *lnfLessEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp <= 0);
}

static lVal *lnfGreater(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp > 0);
}

static lVal *lnfGreaterEqual(lClosure *c, lVal *v){
	const int cmp = lValCompare(c,v);
	return lValBool(cmp == 2 ? false : cmp >= 0);
}

static lVal *lnfNilPred(lClosure *c, lVal *v){
	(void)c;
	lVal *t = lCar(v);
	return lValBool(t == NULL);
}

void lOperationsPredicate(lClosure *c){
	lAddNativeFunc(c,"less? <",           "[a b]","#t if A < B",  lnfLess);
	lAddNativeFunc(c,"less-equal? <=",    "[a b]","#t if A <= B", lnfLessEqual);
	lAddNativeFunc(c,"equal? eqv? eq? =", "[a b]","#t if A == B", lnfEqual);
	lAddNativeFunc(c,"greater-equal? >=", "[a b]","#t if A >= B", lnfGreaterEqual);
	lAddNativeFunc(c,"greater? >",        "[a b]","#t if A > B",  lnfGreater);
	lAddNativeFunc(c,"nil?",              "[a]","#t if A #nil",   lnfNilPred);
}
