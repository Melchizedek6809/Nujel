/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "allocation.h"

#include "../api.h"

/* Handler for [memory-info] */
static lVal *lnfMemInfo(lClosure *c, lVal *v){
	(void)c; (void)v;
	lVal *ret = lRootsValPush(lCons(NULL,NULL));
	lVal *l = ret;

	l->vList.car = lValSym(":value");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lValActive);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

	l->vList.car = lValSym(":closure");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lClosureActive);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

	l->vList.car = lValSym(":array");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lArrayActive);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

	l->vList.car = lValSym(":string");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lStringActive);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

	l->vList.car = lValSym(":symbol");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lSymbolMax);

	return ret;
}

void lOperationsAllocation(lClosure *c){
	lAddNativeFunc(c,"memory-info", "[]", "Return memory usage data", lnfMemInfo);
}
