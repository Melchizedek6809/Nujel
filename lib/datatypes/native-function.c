/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "native-function.h"
#include "../reader.h"

lNFunc   lNFuncList[NFN_MAX];
uint     lNFuncActive = 0;
uint     lNFuncMax    = 1;
uint     lNFuncFFree  = 0;

void lInitNativeFunctions(){
	lNFuncActive = 0;
	lNFuncMax    = 1;
}

void lNFuncFree(uint i){
	if((i == 0) || (i >= lNFuncMax)){return;}
	lNFunc *nfn = &lNFuncList[i];
	if(nfn->nextFree != 0){return;}
	lNFuncActive--;
	nfn->fp       = NULL;
	nfn->doc      = NULL;
	nfn->nextFree = lNFuncFFree;
	nfn->flags    = 0;
	lNFuncFFree   = i;
}

uint lNFuncAlloc(){
	lNFunc *ret;
	if(lNFuncFFree == 0){
		if(lNFuncMax >= NFN_MAX-1){
			lPrintError("lNFunc OOM ");
			return 0;
		}
		ret = &lNFuncList[lNFuncMax++];
	}else{
		ret = &lNFuncList[lNFuncFFree & NFN_MASK];
		lNFuncFFree = ret->nextFree;
	}
	lNFuncActive++;
	*ret = (lNFunc){0};
	return ret - lNFuncList;
}

lVal *lAddNativeFunc(lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *)){
	lVal *lNF = lValNativeFunc(func,lRead(args),lValString(doc));
	return lDefineAliased(c,lNF,sym);
}

lVal *lAddSpecialForm(lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *)){
	lVal *lNF = lValNativeFunc(func,lRead(args),lValString(doc));
	lNF->type = ltSpecialForm;
	return lDefineAliased(c,lNF,sym);
}
