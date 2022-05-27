 /* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "allocator.h"
#include "../printer.h"
#include "../type/val.h"

#include <stdlib.h>
#include <string.h>

bool lGCShouldRunSoon = false;

lArray   lArrayList[ARR_MAX];
uint     lArrayActive = 0;
uint     lArrayMax    = 0;
lArray  *lArrayFFree  = NULL;

lClosure  lClosureList[CLO_MAX];
uint      lClosureActive = 0;
uint      lClosureMax    = 0;
lClosure *lClosureFFree  = NULL;

lString  lStringList[STR_MAX];
uint     lStringActive = 0;
uint     lStringMax    = 0;
lString *lStringFFree  = NULL;

lTree    lTreeList[TRE_MAX];
uint     lTreeActive = 0;
uint     lTreeMax    = 0;
lTree   *lTreeFFree  = NULL;

lVal     lValList[VAL_MAX];
uint     lValActive = 0;
uint     lValMax    = 0;
lVal    *lValFFree  = NULL;

lBytecodeArray  lBytecodeArrayList[BCA_MAX];
uint            lBytecodeArrayActive = 0;
uint            lBytecodeArrayMax    = 0;
lBytecodeArray *lBytecodeArrayFFree  = NULL;

lNFunc   lNFuncList[NFN_MAX];
uint     lNFuncMax    = 0;


#define defineAllocator(T, funcName, list, listMax, listActive, typeMax, listFree, errorMsg) \
T * funcName (){\
	T *ret;\
	if(listFree == NULL){\
		if(listMax >= typeMax-1){\
			fpf(stderr, "%S", errorMsg);\
			exit(123);\
		}else{\
			ret = &list[listMax++];\
		}\
	}else{\
		ret = listFree;\
		listFree = ret->nextFree;\
	}\
	listActive++;\
	if((typeMax - listActive) < 32){lGCShouldRunSoon = true;}\
	memset(ret, 0, sizeof(T));\
	return ret;\
}

defineAllocator(lArray, lArrayAllocRaw, lArrayList, lArrayMax, lArrayActive, ARR_MAX, lArrayFFree, "lArray OOM")
defineAllocator(lClosure, lClosureAlloc, lClosureList, lClosureMax, lClosureActive, CLO_MAX, lClosureFFree, "lClosure OOM")
defineAllocator(lString, lStringAlloc, lStringList, lStringMax, lStringActive, STR_MAX, lStringFFree, "lString OOM")
defineAllocator(lTree, lTreeAlloc, lTreeList, lTreeMax, lTreeActive, TRE_MAX, lTreeFFree, "lTree OOM")
defineAllocator(lVal, lValAllocRaw, lValList, lValMax, lValActive, VAL_MAX, lValFFree, "lVal OOM")
defineAllocator(lBytecodeArray, lBytecodeArrayAllocRaw, lBytecodeArrayList, lBytecodeArrayMax, lBytecodeArrayActive, BCA_MAX, lBytecodeArrayFFree, "lBytecodeArray OOM")

lBytecodeArray *lBytecodeArrayAlloc(size_t len){
	lBytecodeArray *ret = lBytecodeArrayAllocRaw();
	ret->data = calloc(len, sizeof(lBytecodeOp));
	if(ret->data == NULL){
		lExceptionThrowValClo("out-of-memory","Couldn't allocate a new BC array", lValInt(len), NULL);
	}
	ret->dataEnd = &ret->data[len];
	return ret;
}

void lBytecodeArrayFree(lBytecodeArray *v){
	if(v == NULL){return;}
	free(v->data);
	v->data     = NULL;
	v->nextFree = lBytecodeArrayFFree;
	lBytecodeArrayActive--;
	lBytecodeArrayFFree = v;
}

lArray *lArrayAlloc(size_t len){
	lArray *ret = lArrayAllocRaw();
	ret->data = calloc(len, sizeof(lVal *));
	if(ret->data == NULL){
		lExceptionThrowValClo("out-of-memory","Couldn't allocate a new array", lValInt(len), NULL);
	}
	ret->length = len;
	return ret;
}

lNFunc *lNFuncAlloc(){
	if(lNFuncMax >= NFN_MAX-1){
		fpf(stderr, "lNFunc OOM ");
		exit(123);
	}
	return &lNFuncList[lNFuncMax++];
}

void lValFree(lVal *v){
	v->nextFree = lValFFree;
	lValFFree   = v;
	lValActive--;
}

void lArrayFree(lArray *v){
	if(v == NULL){return;}
	free(v->data);
	v->data     = NULL;
	v->nextFree = lArrayFFree;
	lArrayFFree = v;
	lArrayActive--;
}

void lClosureFree(lClosure *clo){
	if(clo == NULL){return;}
	clo->nextFree = lClosureFFree;
	lClosureFFree = clo;
	lClosureActive--;
}

void lStringFree(lString *s){
	if(s == NULL){return;}
	if(s->flags & HEAP_ALLOCATED){
		free((void *)s->buf);
	}
	s->flags = 0;
	s->nextFree = lStringFFree;
	lStringFFree = s;
	lStringActive--;
}

void lTreeFree(lTree *t){
	if(t == NULL){return;}
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
	lTreeActive--;
}

lVal *lValAlloc(lType t){
	lVal *ret = lValAllocRaw();
	ret->type = t;
	return ret;
}
