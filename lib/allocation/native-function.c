/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "native-function.h"
#include "val.h"
#include "../display.h"
#include "../allocation/val.h"
#include "../collection/list.h"
#include "../s-expression/reader.h"
#include "../type/closure.h"

#include <string.h>

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
	memset(ret, 0, sizeof(lNFunc));
	return ret;
}
