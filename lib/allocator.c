/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

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
		(T##FFree) = ret->nextFree;\
	}\
	if(unlikely((typeMax - (++T##Active) < 128))){lGCShouldRunSoon = true;} \
	T##MarkMap[ret - T##List] = 0;\
	memset(ret,0,sizeof(T));\
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

lBufferView *lBufferViewAlloc(lBuffer *buf, lBufferViewType type, size_t offset, size_t length, bool immutable){
	lBufferView *ret = lBufferViewAllocRaw();
	ret->buf    = buf;
	ret->offset = offset;
	ret->length = length;
	ret->flags  = immutable;
	ret->type   = type;
	return ret;
}

lBytecodeArray *lBytecodeArrayAlloc(size_t len){
	lBytecodeArray *ret = lBytecodeArrayAllocRaw();
	ret->data = calloc(len, sizeof(lBytecodeOp));
	if(unlikely(ret->data == NULL)){
		fprintf(stderr, "OOM: Couldn't allocate a new BC array\n");
		exit(134);
	}
	ret->dataEnd = &ret->data[len];
	return ret;
}

lArray *lArrayAlloc(size_t len){
	lArray *ret = lArrayAllocRaw();
	ret->data = calloc(len + 1, sizeof(lVal));
	if(unlikely(ret->data == NULL)){
		fprintf(stderr, "OOM: Couldn't allocate a new array");
		exit(135);
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
