/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"

#include "../api.h"
#include "../type/bytecode.h"

#include <stdlib.h>

static lVal *lValBytecodeOp(lBytecodeOp v){
	lVal *ret = lValAlloc();
	ret->type = ltBytecodeOp;
	ret->vBytecodeOp = v;
	return ret;
}

static lVal *lnfIntBytecodeOp(lClosure *c, lVal *v){
	const i64 val = castToInt(lCar(v), -1);
	if((val < 0) || (val > 255)){
		lExceptionThrowValClo(":invalid-bc-op", "Bytecode operations have to be within the range 0-255", lCar(v), c);
		return NULL;
	}
	return lValBytecodeOp(val);
}

static lVal *lnfBytecodeOpInt(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	if((car == NULL) || (car->type != ltBytecodeOp)){
		lExceptionThrowValClo(":argument-mismatch", "Expected 1 argument of type :bytecode-op", v, c);
		return NULL;
	}
	return lValInt(car->vBytecodeOp);
}

static lVal *lnfArrBytecodeArr(lClosure *c, lVal *v){
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){
		lExceptionThrowValClo(":argument-mismatch", "Expected 1 argument of type :array", v, c);
		return NULL;
	}
	const int len = arr->vArray->length;
	lVal *ret = lValAlloc();
	ret->type = ltBytecodeArr;
	ret->vBytecodeArr.data = malloc(len * sizeof(lBytecodeOp));
	ret->vBytecodeArr.dataEnd = &ret->vBytecodeArr.data[len];

	for(int i=0;i<len;i++){
		const lVal *item = arr->vArray->data[i];
		if((item == NULL) || (item->type != ltBytecodeOp)){
			lExceptionThrowValClo(":argument-mismatch", "Expected array to only contain values of type :bytecode-op", v, c);
			return NULL;
		}
		ret->vBytecodeArr.data[i] = item->vBytecodeOp;
	}
	return ret;
}

static lVal *lnfBytecodeArrArr(lClosure *c, lVal *v){
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltBytecodeArr)){
		lExceptionThrowValClo(":argument-mismatch", "Expected 1 argument of type :array", v, c);
		return NULL;
	}
	const int len = arr->vBytecodeArr.dataEnd - arr->vBytecodeArr.data;
	lVal *ret = RVP(lValAlloc());
	ret->type = ltArray;
	ret->vArray = lArrayAlloc();
	ret->vArray->length = len;
	ret->vArray->data = malloc(len * sizeof(*ret->vArray->data));

	for(int i=0;i<len;i++){
		ret->vArray->data[i] = lValBytecodeOp(arr->vBytecodeArr.data[i]);
	}
	return ret;
}

static lVal *lnfBytecodeEval(lClosure *c, lVal *v){
	lVal *opsArr = lCar(v);
	if((opsArr == NULL) || (opsArr->type != ltBytecodeArr)){
		lExceptionThrowValClo(":argument-mismatch", "Expected first argument to be of type :bytecode-array", v, c);
		return NULL;
	}
	return lBytecodeEval(c, lCdr(v), &opsArr->vBytecodeArr);
}

void lOperationsBytecode(lClosure *c){
	lAddNativeFunc(c,"int->bytecode-op",  "[a]", "Turns an integer into a bytecode operation with the same value", lnfIntBytecodeOp);
	lAddNativeFunc(c,"bytecode-op->int",  "[a]", "Turns a bytecode operation into an integer of the same value", lnfBytecodeOpInt);
	lAddNativeFunc(c,"arr->bytecode-arr", "[a]", "Turns an array of bytecode operations into a bytecode array", lnfArrBytecodeArr);
	lAddNativeFunc(c,"bytecode-arr->arr", "[a]", "Turns an bytecode array into an array of bytecode operations", lnfBytecodeArrArr);
	lAddNativeFunc(c,"bytecode-eval",     "[bc args]", "Evaluate a bytecode array and return the result", lnfBytecodeEval);
}
