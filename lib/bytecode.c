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

static lVal lnfArrBytecodeArr(lClosure *c, lVal v){
	(void)c;
	lVal car = requireArray(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lArray *arr = car.vArray;
	const int len = arr->length;
	lVal cadr = requireArray(lCadr(v));
	if(unlikely(cadr.type == ltException)){
		return cadr;
	}
	lArray *literals = cadr.vArray;
	lBytecodeOp *ops = malloc(sizeof(lBytecodeOp) * len);
	for(int i=0;i<len;i++){
		lVal t = requireInt(arr->data[i]);
		if(unlikely(t.type == ltException)){
			return t;
		}
		ops[i] = t.vInt;
	}
	lVal ret = lValBytecodeArray(ops,len,literals);
	free(ops);
	return ret;
}

static lVal lnfBytecodeArrArr(lClosure *c, lVal v){
	(void)c;
	lVal car = requireBytecodeArray(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBytecodeArray *arr = car.vBytecodeArr;
	const int len = arr->dataEnd - arr->data;

	lVal ret = lValAlloc(ltArray, lArrayAlloc(len));
	for(int i=0;i<len;i++){
		ret.vArray->data[i] = lValInt(arr->data[i]);
	}
	return ret;
}

static lVal lnfBytecodeLiterals(lClosure *c, lVal v){
	(void)c;
	lVal car = requireBytecodeArray(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBytecodeArray *arr = car.vBytecodeArr;
	if(unlikely(arr->literals == NULL)){return NIL;}
	return lValAlloc(ltArray, arr->literals);
}

static lVal lnfBytecodeArrLength(lClosure *c, lVal v){
	(void)c;
	lVal car = requireBytecodeArray(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBytecodeArray *arr = car.vBytecodeArr;
	return lValInt(arr->dataEnd - arr->data);
}

void lOperationsBytecode(lClosure *c){
	lAddNativeFunc(c,"arr->bytecode-arr",    "(a)", "Turns an array of bytecode operations into a bytecode array", lnfArrBytecodeArr);
	lAddNativeFunc(c,"bytecode-arr->arr",    "(a)", "Turns an bytecode array into an array of bytecode operations", lnfBytecodeArrArr);
	lAddNativeFunc(c,"bytecode-literals",    "(a)", "Return the literal section of a BCA", lnfBytecodeLiterals);

	lAddNativeFunc(c,"bytecode-array/length","(a)", "Return the length of the bytecode-array A", lnfBytecodeArrLength);
}
