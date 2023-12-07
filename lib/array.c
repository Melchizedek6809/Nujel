/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

static lVal lnmArrayLength(lVal self){
	return lValInt(self.vArray->length);
}

static lVal lnmArrayLengthSet(lVal self, lVal newLength){
	reqNaturalInt(newLength);
	const size_t length = newLength.vInt;
	lArray *arr = self.vArray;

	lVal *newData = realloc(arr->data,length * sizeof(lVal));
	if (unlikely(newData == NULL)) {
		free(newData);
		return lValException(lSymOOM, "(:length Array) couldn't allocate its array", self);
	}
	arr->data = newData;
	if(length > (size_t)arr->length){
		memset(&arr->data[arr->length], 0, (length - arr->length) * sizeof(lVal));
	}
	arr->length = length;
	return self;
}

/* Return the length of the list V */
static int lListLength(lVal v){
	int i = 0;
	for(lVal n = v;(n.type == ltPair) && (n.vList->car.type != ltNil); n = n.vList->cdr){
		i++;
	}
	return i;
}

lVal lnfArrNew(lVal v){
	int length = lListLength(v);
	lVal r = lValAlloc(ltArray, lArrayAlloc(length));
	int key = 0;
	for(lVal n = v; n.type == ltPair; n = n.vList->cdr){
		r.vArray->data[key++] = n.vList->car;
	}
	return r;
}

static lVal lnmArrayToBytecodeArray(lVal self, lVal aLiterals){
	lArray *arr = self.vArray;
	const int len = arr->length;

	reqArray(aLiterals);
	lBytecodeOp *ops = malloc(sizeof(lBytecodeOp) * len);
	for(int i=0;i<len;i++){
		if(unlikely(arr->data[i].type != ltInt)){
			free(ops);
			return lValException(lSymTypeError, "Need an Int", arr->data[i]);
		}
		ops[i] = arr->data[i].vInt;
	}
	lVal ret = lValBytecodeArray(ops, len, aLiterals.vArray);
	free(ops);
	return ret;
}

static lVal lnmArrayAllocate(lVal self, lVal size){
	(void)self;
	reqNaturalInt(size);
	lVal r = lValAlloc(ltArray, lArrayAlloc(size.vInt));
	if(unlikely(size.vInt && (r.vArray->data == NULL))){
		return lValException(lSymOOM, "(:alloc Array) couldn't allocate its array", size);
	}
	return r;
}

static lVal lnmArrayHas(lVal self, lVal index){
	reqInt(index);
	const i64 i = index.vInt;
	return lValBool((i >= 0) && (i < self.vArray->length));
}

void lOperationsArray(){
	lClass *Array = &lClassList[ltArray];
	lAddNativeMethodV(Array,  lSymS("length"),  "(self)", lnmArrayLength, 0);
	lAddNativeMethodVV(Array, lSymS("length!"), "(self new-size)", lnmArrayLengthSet, 0);
	lAddNativeMethodVV(Array, lSymS("bytecode-array"), "(self literals)", lnmArrayToBytecodeArray, 0);
	lAddNativeMethodVV(Array, lSymS("has?"),   "(self index)", lnmArrayHas, NFUNC_PURE);

	lAddNativeStaticMethodVV(Array, lSymS("alloc"), "(self size)", lnmArrayAllocate, NFUNC_PURE);
}
