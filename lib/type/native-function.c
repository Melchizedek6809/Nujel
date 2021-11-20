/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "native-function.h"
#include "val.h"
#include "../display.h"
#include "../allocation/native-function.h"
#include "../allocation/val.h"
#include "../collection/list.h"
#include "../s-expression/reader.h"
#include "../type/closure.h"

static lVal *lValNativeFunc(lVal *(*func)(lClosure *,lVal *), lVal *args, lVal *docString){
	lVal *v = lRootsValPush(lValAlloc());
	v->type   = ltNativeFunc;
	v->vNFunc = lNFuncAlloc();

	v->vNFunc->fp   = func;
	v->vNFunc->doc  = docString;
	v->vNFunc->args = args;
	return v;
}

lVal *lAddNativeFunc(lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *)){
	lVal *lNF = lValNativeFunc(func,lCar(lRead(args)),lValString(doc));
	return lDefineAliased(c,lNF,sym);
}

lVal *lAddSpecialForm(lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *)){
	lVal *lNF = lValNativeFunc(func,lCar(lRead(args)),lValString(doc));
	lNF->type = ltSpecialForm;
	return lDefineAliased(c,lNF,sym);
}

int lNFuncID(const lNFunc *n){
	return n - lNFuncList;
}
