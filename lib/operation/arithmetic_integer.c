/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"


#include "../exception.h"
#include "../nujel.h"
#include "../type-system.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/val.h"

#include <math.h>

static lVal *lnfAddAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a + b);
}

static lVal *lnfSubAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a - b);
}

static lVal *lnfMulAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a * b);
}

static lVal *lnfDivAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a / b);
}

static lVal *lnfModAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a % b);
}

static lVal *lnfPowAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(pow(a,b));
}

void lOperationsArithmeticInteger(lClosure *c){
	lAddNativeFunc(c,"add/int", "[a b]", "Return a:int + b:int",  lnfAddAstI);
	lAddNativeFunc(c,"sub/int", "[a b]", "Return a:int - b:int",  lnfSubAstI);
	lAddNativeFunc(c,"mul/int", "[a b]", "Return a:int * b:int",  lnfMulAstI);
	lAddNativeFunc(c,"div/int", "[a b]", "Return a:int / b:int",  lnfDivAstI);
	lAddNativeFunc(c,"mod/int", "[a b]", "Return a:int % b:int",  lnfModAstI);
	lAddNativeFunc(c,"pow/int", "[a b]", "Return a:int ** b:int", lnfPowAstI);
}
