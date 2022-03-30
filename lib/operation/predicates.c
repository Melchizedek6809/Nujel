/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../type/native-function.h"
#include "../type/val.h"

static lVal *lnfLess(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(lCar(v), lCadr(v));
	return lValBool(cmp == 2 ? false : cmp < 0);
}

static lVal *lnfUnequal(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(lCar(v), lCadr(v));
	return lValBool(!(cmp == 2 ? false : cmp == 0));
}

static lVal *lnfEqual(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(lCar(v), lCadr(v));
	return lValBool(cmp == 2 ? false : cmp == 0);
}

static lVal *lnfLessEqual(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(lCar(v), lCadr(v));
	return lValBool(cmp == 2 ? false : cmp <= 0);
}

static lVal *lnfGreater(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(lCar(v), lCadr(v));
	return lValBool(cmp == 2 ? false : cmp > 0);
}

static lVal *lnfGreaterEqual(lClosure *c, lVal *v){
	(void)c;
	const int cmp = lValCompare(lCar(v), lCadr(v));
	return lValBool(cmp == 2 ? false : cmp >= 0);
}

static lVal *lnfNilPred(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lCar(v) == NULL);
}

static lVal *lnfKeywordPred(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	return lValBool(car ? car->type == ltKeyword : false);
}

void lOperationsPredicate(lClosure *c){
	lAddNativeFunc(c,"<",        "[α β]", "Return true if α is less than β",             lnfLess);
	lAddNativeFunc(c,"<=",       "[α β]", "Return true if α is less or equal to β",      lnfLessEqual);
	lAddNativeFunc(c,"==",       "[α β]", "Return true if α is equal to β",              lnfEqual);
	lAddNativeFunc(c,"!=",       "[α β]", "Return true if α is not equal to  β",         lnfUnequal);
	lAddNativeFunc(c,">=",       "[α β]", "Return true if α is greater or equal than β", lnfGreaterEqual);
	lAddNativeFunc(c,">",        "[α β]", "Return true if α is greater than β",          lnfGreater);
	lAddNativeFunc(c,"nil?",     "[α]",   "Return true if α is #nil",                    lnfNilPred);
	lAddNativeFunc(c,"keyword?", "[α]",   "Return true if α is a keyword symbol",        lnfKeywordPred);
}
