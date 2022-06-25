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

lBuffer  lBufferList[BUF_MAX];
uint     lBufferMax = 0;
uint     lBufferActive = 0;
lBuffer *lBufferFFree = NULL;

lBufferView  lBufferViewList[BFV_MAX];
uint         lBufferViewMax = 0;
uint         lBufferViewActive = 0;
lBufferView *lBufferViewFFree = NULL;

lNFunc   lNFuncList[NFN_MAX];
uint     lNFuncMax    = 0;


#define defineAllocator(T, funcName, list, listMax, listActive, typeMax, listFree, errorMsg) \
T * funcName (){\
	T *ret;\
	if(listFree == NULL){\
		if(unlikely(listMax >= typeMax-1)){	\
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
	if(unlikely((typeMax - listActive) < 32)){lGCShouldRunSoon = true;} \
	memset(ret, 0, sizeof(T));\
	return ret;\
}

defineAllocator(lArray, lArrayAllocRaw, lArrayList, lArrayMax, lArrayActive, ARR_MAX, lArrayFFree, "lArray OOM")
defineAllocator(lClosure, lClosureAlloc, lClosureList, lClosureMax, lClosureActive, CLO_MAX, lClosureFFree, "lClosure OOM")
defineAllocator(lTree, lTreeAlloc, lTreeList, lTreeMax, lTreeActive, TRE_MAX, lTreeFFree, "lTree OOM")
defineAllocator(lVal, lValAllocRaw, lValList, lValMax, lValActive, VAL_MAX, lValFFree, "lVal OOM")
defineAllocator(lBytecodeArray, lBytecodeArrayAllocRaw, lBytecodeArrayList, lBytecodeArrayMax, lBytecodeArrayActive, BCA_MAX, lBytecodeArrayFFree, "lBytecodeArray OOM")
defineAllocator(lBuffer, lBufferAllocRaw, lBufferList, lBufferMax, lBufferActive, BUF_MAX, lBufferFFree, "lBuffer OOM")
defineAllocator(lBufferView, lBufferViewAllocRaw, lBufferViewList, lBufferViewMax, lBufferViewActive, BFV_MAX, lBufferViewFFree, "lBufferView OOM")

int lBufferViewTypeSize(lBufferViewType T){
	switch(T){
	default:
		exit(4);
	case lbvtU8:
	case lbvtS8:
		return 1;
	case lbvtS16:
	case lbvtU16:
		return 2;
	case lbvtF32:
	case lbvtS32:
	case lbvtU32:
		return 4;
	case lbvtF64:
	case lbvtS64:
		return 8;
	}
}


lBuffer *lBufferAlloc(size_t length, bool immutable){
	lBuffer *ret = lBufferAllocRaw();
	ret->length = length;
	if(immutable){
		ret->flags = BUFFER_IMMUTABLE;
	}else{
		ret->buf = calloc(length, 1);
	}
	return ret;
}

void lBufferFree(lBuffer *buf){
	if(unlikely(buf == NULL)){return;}
	free(buf->buf);
	buf->nextFree = lBufferFFree;
	lBufferActive--;
	lBufferFFree = buf;
}

lBufferView *lBufferViewAlloc(lBuffer *buf, lBufferViewType type, size_t offset, size_t length, bool immutable){
	lBufferView *ret = lBufferViewAllocRaw();
	ret->buf    = buf;
	ret->offset = offset;
	ret->length = length;
	ret->flags  = immutable;
	ret->type   = type;
	return ret;
}

void lBufferViewFree(lBufferView *buf){
	if(unlikely(buf == NULL)){return;}
	buf->buf = NULL;
	buf->nextFree = lBufferViewFFree;
	lBufferViewActive--;
	lBufferViewFFree = buf;
}

lBytecodeArray *lBytecodeArrayAlloc(size_t len){
	lBytecodeArray *ret = lBytecodeArrayAllocRaw();
	ret->data = calloc(len, sizeof(lBytecodeOp));
	if(unlikely(ret->data == NULL)){
		lExceptionThrowValClo("out-of-memory","Couldn't allocate a new BC array", lValInt(len), NULL);
	}
	ret->dataEnd = &ret->data[len];
	return ret;
}

void lBytecodeArrayFree(lBytecodeArray *v){
	if(unlikely(v == NULL)){return;}
	free(v->data);
	v->data     = NULL;
	v->nextFree = lBytecodeArrayFFree;
	lBytecodeArrayActive--;
	lBytecodeArrayFFree = v;
}

lArray *lArrayAlloc(size_t len){
	lArray *ret = lArrayAllocRaw();
	ret->data = calloc(len, sizeof(lVal *));
	if(unlikely(ret->data == NULL)){
		lExceptionThrowValClo("out-of-memory","Couldn't allocate a new array", lValInt(len), NULL);
	}
	ret->length = len;
	return ret;
}

lNFunc *lNFuncAlloc(){
	if(unlikely(lNFuncMax >= NFN_MAX-1)){
		fpf(stderr, "lNFunc OOM ");
		exit(123);
	}
	memset(&lNFuncList[lNFuncMax++], 0, sizeof(ltNativeFunc));
	return &lNFuncList[lNFuncMax++];
}

void lValFree(lVal *v){
	v->type     = ltNoAlloc;
	v->nextFree = lValFFree;
	lValFFree   = v;
	lValActive--;
}

void lArrayFree(lArray *v){
	if(unlikely(v == NULL)){return;}
	free(v->data);
	v->data     = NULL;
	v->nextFree = lArrayFFree;
	lArrayFFree = v;
	lArrayActive--;
}

void lClosureFree(lClosure *clo){
	if(unlikely(clo == NULL)){return;}
	clo->nextFree = lClosureFFree;
	lClosureFFree = clo;
	lClosureActive--;
}

void lTreeFree(lTree *t){
	if(unlikely(t == NULL)){return;}
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
	lTreeActive--;
}

lVal *lValAlloc(lType t){
	lVal *ret = lValAllocRaw();
	ret->type = t;
	return ret;
}
