/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <string.h>

lVal lValBytecodeArray(const lBytecodeOp *ops, int opsLength, lArray *literals){
	lVal ret = lValAlloc(ltBytecodeArr, lBytecodeArrayAlloc(opsLength));
	ret.vBytecodeArr->literals = literals;
	ret.vBytecodeArr->literals->flags |= ARRAY_IMMUTABLE;
	memcpy(ret.vBytecodeArr->data, ops, opsLength);
	return ret;
}

static lVal lnmBytecodeArrayArray(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	const int len = arr->dataEnd - arr->data;

	lVal ret = lValAlloc(ltArray, lArrayAlloc(len));
	for(int i=0;i<len;i++){
		ret.vArray->data[i] = lValInt(arr->data[i]);
	}
	return ret;
}

static lVal lnmBytecodeArrayLiterals(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	if(unlikely(arr->literals == NULL)){
		return NIL;
	} else {
		return lValAlloc(ltArray, arr->literals);
	}
}

static lVal lnmBytecodeArrayLength(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	return lValInt(arr->dataEnd - arr->data);
}

void lOperationsBytecode(lClosure *c){
	(void)c;
	lClass *BytecodeArray = &lClassList[ltBytecodeArr];
	lAddNativeMethodV(BytecodeArray, lSymS("array"),    "(self)", lnmBytecodeArrayArray, 0);
	lAddNativeMethodV(BytecodeArray, lSymS("literals"), "(self)", lnmBytecodeArrayLiterals, 0);
	lAddNativeMethodV(BytecodeArray, lSymS("length"),   "(self)", lnmBytecodeArrayLength, 0);
}
