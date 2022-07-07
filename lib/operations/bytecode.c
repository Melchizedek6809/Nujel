/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../allocation/symbol.h"
#include "../printer.h"
#include "../type-system.h"
#include "../type/closure.h"
#include "../type/val.h"
#include "../vm/eval.h"

static lVal *lValBytecodeOp(lBytecodeOp v){
	lVal *ret = lValAlloc(ltBytecodeOp);
	ret->vBytecodeOp = v;
	return ret;
}

static lVal *lnfIntBytecodeOp(lClosure *c, lVal *v){
	const i64 val = requireInt(c, lCar(v));
	if((val < -128) || (val > 255)){
		lExceptionThrowValClo("invalid-bc-op", "Bytecode operations have to be within the range -128 - 255", lCar(v), c);
		return NULL;
	}
	return lValBytecodeOp(val);
}

static lVal *lnfBytecodeOpInt(lClosure *c, lVal *v){
	return lValInt(requireBytecodeOp(c, lCar(v)));
}

static lVal *lnfArrBytecodeArr(lClosure *c, lVal *v){
	lArray *arr = requireArray(c, lCar(v));
	const int len = arr->length;

	lVal *litv = lCadr(v);
	lArray *literals = NULL;
	if(litv){
		literals = requireArray(c, litv);
	}

	lVal *ret = lValAlloc(ltBytecodeArr);
	lBytecodeArray *bca = lBytecodeArrayAlloc(len);
	ret->vBytecodeArr = bca;
	bca->literals = literals;

	for(int i=0;i<len;i++){
		bca->data[i] = requireBytecodeOp(c, arr->data[i]);
	}

	return ret;
}

static lVal *lnfBytecodeArrArr(lClosure *c, lVal *v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));
	const int len = arr->dataEnd - arr->data;

	lVal *ret = lValAlloc(ltArray);
	ret->vArray = lArrayAlloc(len);

	for(int i=0;i<len;i++){
		ret->vArray->data[i] = lValBytecodeOp(arr->data[i]);
	}

	return ret;
}

static lVal *lnfBytecodeLiterals(lClosure *c, lVal *v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));

	lVal *ret = lValAlloc(ltArray);
	ret->vArray = arr->literals;

	return ret;
}

void lOperationsBytecode(lClosure *c){
	lAddNativeFuncPure(c,"int->bytecode-op",  "[a]", "Turns an integer into a bytecode operation with the same value", lnfIntBytecodeOp);
	lAddNativeFuncPure(c,"bytecode-op->int",  "[a]", "Turns a bytecode operation into an integer of the same value", lnfBytecodeOpInt);
	lAddNativeFunc(c,"arr->bytecode-arr", "[a]", "Turns an array of bytecode operations into a bytecode array", lnfArrBytecodeArr);
	lAddNativeFunc(c,"bytecode-arr->arr", "[a]", "Turns an bytecode array into an array of bytecode operations", lnfBytecodeArrArr);
	lAddNativeFunc(c,"bytecode-literals", "[a]", "Return the literal section of a BCA", lnfBytecodeLiterals);
}
