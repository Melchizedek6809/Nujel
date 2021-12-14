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
	return lList(10,
		RVP(lValSym(":value")),  RVP(lValInt(lValActive)),
		RVP(lValSym(":closure")),RVP(lValInt(lClosureActive)),
		RVP(lValSym(":array")),  RVP(lValInt(lArrayActive)),
		RVP(lValSym(":string")), RVP(lValInt(lStringActive)),
		RVP(lValSym(":symbol")), RVP(lValInt(lSymbolMax)));
}

static lVal *lnfGarbageCollect(lClosure *c, lVal *v){
	(void)c; (void)v;
	lGarbageCollect();
	return NULL;
}

void lOperationsAllocation(lClosure *c){
	lAddNativeFunc(c,"memory-info", "[]", "Return memory usage data", lnfMemInfo);
	lAddNativeFunc(c,"garbage-collect", "[]", "Force the garbage collector to run", lnfGarbageCollect);
}
