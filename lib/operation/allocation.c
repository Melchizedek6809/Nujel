/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"

#include "../api.h"


/* Handler for [memory-info] */
static lVal *lnfMemInfo(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lList(10,
		RVP(lValSym(":value")),  RVP(lValInt(lValActive)),
		RVP(lValSym(":closure")),RVP(lValInt(lClosureActive)),
		RVP(lValSym(":array")),  RVP(lValInt(lArrayActive)),
		RVP(lValSym(":string")), RVP(lValInt(lStringActive)),
		RVP(lValSym(":symbol")), RVP(lValInt(lSymbolActive)));
}

static lVal *lnfGarbageCollect(lClosure *c, lVal *v){
	(void)c; (void)v;
	lGarbageCollect();
	return NULL;
}

static lVal *lnfValIndex(lClosure *c, lVal *v){
	(void)c;
	return lValInt(lValIndex(lCar(v)));
}

static lVal *lnfIndexVal(lClosure *c, lVal *v){
	(void)c;
	const int i = castToInt(lCar(v), -1);
	return lIndexVal(i);
}

void lOperationsAllocation(lClosure *c){
	lAddNativeFunc(c,"memory-info", "[]", "Return memory usage data", lnfMemInfo);
	lAddNativeFunc(c,"garbage-collect", "[]", "Force the garbage collector to run", lnfGarbageCollect);
	lAddNativeFunc(c,"val->index", "[v]", "Return an index value pointing to V", lnfValIndex);
	lAddNativeFunc(c,"index->val", "[i]", "Return an the value at index position I", lnfIndexVal);
}
