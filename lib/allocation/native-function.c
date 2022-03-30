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

/* Initialize the NFunc allocator */
void lNativeFunctionsInit(){
	lNFuncActive = 0;
	lNFuncMax    = 0;
}

/* Allocate a new NFunc */
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

/* Return the Index of NFunc n */
int lNFuncID(const lNFunc *n){
	return n - lNFuncList;
}
