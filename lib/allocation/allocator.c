/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "allocator.h"
#include "../display.h"

#include <stdlib.h>
#include <string.h>

lArray   lArrayList[ARR_MAX];
uint     lArrayMax    = 0;
lArray  *lArrayFFree  = NULL;

lClosure  lClosureList[CLO_MAX];
uint      lClosureMax    = 0;
lClosure *lClosureFFree  = NULL;

lNFunc   lNFuncList[NFN_MAX];
uint     lNFuncMax    = 0;

lString  lStringList[STR_MAX];
uint     lStringMax    = 0;
lString *lStringFFree  = NULL;

lTree    lTreeList[TRE_MAX];
uint     lTreeMax    = 0;
lTree   *lTreeFFree  = NULL;

lVal     lValList[VAL_MAX];
uint     lValMax    = 0;
lVal    *lValFFree  = NULL;

#define defineAllocator(T, funcName, list, listMax, typeMax, listFree, errorMsg) \
T * funcName (){\
	T *ret;\
	if(listFree == NULL){\
		if(listMax >= typeMax-1){\
			lGarbageCollect();\
			if(listFree == NULL){\
				lPrintError(errorMsg);\
				exit(123);\
			}else{\
				goto allocateFromFreeList;\
			}\
		}else{\
			ret = &list[listMax++];\
		}\
	}else{\
		allocateFromFreeList:\
		ret = listFree;\
		listFree = ret->nextFree;\
	}\
	memset(ret, 0, sizeof(T));\
	return ret;\
}

defineAllocator(lArray, lArrayAlloc, lArrayList, lArrayMax, ARR_MAX, lArrayFFree, "lArray OOM")
defineAllocator(lClosure, lClosureAlloc, lClosureList, lClosureMax, CLO_MAX, lClosureFFree, "lClosure OOM")
defineAllocator(lString, lStringAlloc, lStringList, lStringMax, STR_MAX, lStringFFree, "lString OOM")
defineAllocator(lTree, lTreeAlloc, lTreeList, lTreeMax, TRE_MAX, lTreeFFree, "lTree OOM")
defineAllocator(lVal, lValAllocRaw, lValList, lValMax, VAL_MAX, lValFFree, "lVal OOM")

lNFunc *lNFuncAlloc(){
	if(lNFuncMax >= NFN_MAX-1){
		lPrintError("lNFunc OOM ");
		exit(123);
	}
	return &lNFuncList[lNFuncMax++];
}

void lValFree(lVal *v){
	if(v->type == ltBytecodeArr){
		free((void *)v->vBytecodeArr.data);
		v->vBytecodeArr.data = NULL;
	}
	v->nextFree = lValFFree;
	lValFFree   = v;
}

void lArrayFree(lArray *v){
	if(v == NULL){return;}
	free(v->data);
	v->data     = NULL;
	v->nextFree = lArrayFFree;
	lArrayFFree = v;
}

void lClosureFree(lClosure *clo){
	if(clo == NULL){return;}
	clo->nextFree = lClosureFFree;
	lClosureFFree = clo;
}

void lStringFree(lString *s){
	if(s == NULL){return;}
	if(s->flags & HEAP_ALLOCATED){
		free((void *)s->buf);
	}
	s->flags = 0;
	s->nextFree = lStringFFree;
	lStringFFree = s;
}

void lTreeFree(lTree *t){
	if(t == NULL){return;}
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
}
