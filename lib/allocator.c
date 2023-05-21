/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <stdlib.h>
#include <string.h>

bool lGCShouldRunSoon = false;

#define defineAllocator(T, typeMax) \
T T##List[typeMax]; \
uint T##Max = 0; \
uint T##Active = 0; \
T * T##FFree = NULL; \
T * T##AllocRaw (){\
	T *ret;\
	if((T##FFree) == NULL){			\
		if(unlikely(T##Max >= typeMax-1)){	\
			fprintf(stderr, "OOM: static %s heap exhausted \n", #T);\
			exit(123);\
		}else{\
			ret = &(T##List)[(T##Max)++];	\
		}\
	}else{\
		ret = T ## FFree;\
		(T##FFree) = ret->nextFree;	\
	}\
	(T##Active)++;							\
	if(unlikely((typeMax - (T##Active)) < 256)){lGCShouldRunSoon = true;} \
	*ret = (T){0};\
	return ret;\
}
allocatorTypes()
#undef defineAllocator

lNFunc   lNFuncList[NFN_MAX];
uint     lNFuncMax    = 0;


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
		lExceptionThrowValClo("out-of-memory", "Couldn't allocate a new BC array", lValInt(len), NULL);
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
	ret->data = calloc(len, sizeof(lVal));
	if(unlikely(ret->data == NULL)){
		lExceptionThrowValClo("out-of-memory", "Couldn't allocate a new array", lValInt(len), NULL);
	}
	ret->length = len;
	return ret;
}

lNFunc *lNFuncAlloc(){
	if(unlikely(lNFuncMax >= NFN_MAX-1)){
		exit(124);
	}
	memset(&lNFuncList[lNFuncMax++], 0, sizeof(ltNativeFunc));
	return &lNFuncList[lNFuncMax++];
}

void lNFuncFree(lNFunc *n){
	(void)n;
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

void lTreeRootFree(lTreeRoot *t){
	if(unlikely(t == NULL)){return;}
	t->nextFree = lTreeRootFFree;
	lTreeRootFFree = t;
	lTreeRootActive--;
}

void lPairFree(lPair *cons){
	if(unlikely(cons == NULL)){return;}
	cons->car = NIL;
	cons->nextFree = lPairFFree;
	lPairFFree = cons;
	lPairActive--;
}
