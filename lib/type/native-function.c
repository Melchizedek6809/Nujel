/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "native-function.h"
#include "val.h"
#include "../display.h"
#include "../allocation/val.h"
#include "../collection/closure.h"
#include "../collection/list.h"
#include "../s-expression/reader.h"

lNFunc   lNFuncList[NFN_MAX];
uint     lNFuncActive = 0;
uint     lNFuncMax    = 0;
lNFunc  *lNFuncFFree  = NULL;

void lNativeFunctionsInit(){
	lNFuncActive = 0;
	lNFuncMax    = 0;
}

void lNFuncFree(uint i){
	if((i == 0) || (i >= lNFuncMax)){return;}
	lNFunc *nfn = &lNFuncList[i];
	if(nfn->nextFree != 0){return;}
	lNFuncActive--;
	nfn->fp       = NULL;
	nfn->doc      = NULL;
	nfn->nextFree = lNFuncFFree;
	lNFuncFFree   = nfn;
}

lNFunc *lNFuncAlloc(){
	lNFunc *ret;
	if(lNFuncFFree == NULL){
		if(lNFuncMax >= NFN_MAX-1){
			lPrintError("lNFunc OOM ");
			return 0;
		}
		ret = &lNFuncList[lNFuncMax++];
	}else{
		ret = lNFuncFFree;
		lNFuncFFree = ret->nextFree;
	}
	lNFuncActive++;
	*ret = (lNFunc){0};
	return ret;
}

static lVal *lValNativeFunc(lVal *(*func)(lClosure *,lVal *), lVal *args, lVal *docString){
	lVal *v = lValAlloc();
	if(v == NULL){return NULL;}
	v->type    = ltNativeFunc;
	lNFunc *fn = lNFuncAlloc();
	if(fn == NULL){
		return NULL;
	}else{
		fn->fp     = func;
		fn->doc    = lCons(args,docString);
		v->vNFunc  = fn;
		return v;
	}
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
