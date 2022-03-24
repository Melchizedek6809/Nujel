/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "native-function.h"
#include "../display.h"

#include <string.h>
#include <stdlib.h>

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
			exit(123);
		}
		ret = &lNFuncList[lNFuncMax++];
	}else{
		ret = lNFuncFFree;
		lNFuncFFree = ret->nextFree;
	}
	lNFuncActive++;
	memset(ret, 0, sizeof(lNFunc));
	return ret;
}
