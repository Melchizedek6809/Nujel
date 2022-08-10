/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

#include <string.h>

static bool lBytecodeArrayCheckIfValid(const lBytecodeOp *ops, int opsLength, lArray *literals){
	(void)ops;
	(void)opsLength;
	(void)literals;
	// ToDo: check that all literal accesses are in bounds
	// ToDo: check that IP is always within bounds
	// ToDo: check that SP is always within bounds
	// ToDo: check that all bytecodes are valid
	// ToDo: check maximum (C)SP size
	// ToDo: check that the last opcode is always a return
	// ToDo: check get/def/set symbol literal types
	return true;
}

lVal *lValBytecodeArray(const lBytecodeOp *ops, int opsLength, lArray *literals, lClosure *errorClosure){
	if(!lBytecodeArrayCheckIfValid(ops, opsLength, literals)){
		lExceptionThrowValClo("invalid-bc-array", "Invalid bytecode array", NULL, errorClosure);
		return NULL;
	}
	lVal *ret = lValAlloc(ltBytecodeArr);
	ret->vBytecodeArr = lBytecodeArrayAlloc(opsLength);
	ret->vBytecodeArr->literals = literals;
	ret->vBytecodeArr->literals->flags |= ARRAY_IMMUTABLE;
	memcpy(ret->vBytecodeArr->data, ops, opsLength);
	return ret;
}

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
	lArray *literals = requireArray(c, lCadr(v));
	lBytecodeOp *ops = malloc(sizeof(lBytecodeOp) * len);
	for(int i=0;i<len;i++){
		ops[i] = requireBytecodeOp(c, arr->data[i]);
	}
	if(!lBytecodeArrayCheckIfValid(ops, len, literals)){
		lExceptionThrowValClo("invalid-bc-array", "The bytecodes and literal array are invalid", v, c);
		return NULL;
	}
	lVal *ret = lValBytecodeArray(ops,len,literals,c);
	free(ops);
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
	if(arr->literals == NULL){return NULL;}
	lVal *ret = lValAlloc(ltArray);
	ret->vArray = arr->literals;

	return ret;
}

static lVal *lnfBytecodeArrRef(lClosure *c, lVal *v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));
	const i64 i = requireNaturalInt(c, lCadr(v));
	if(i >= (arr->dataEnd - arr->data)){
		lExceptionThrowValClo("out-of-bounds", "Bytecode array ref was out of bounds", lCadr(v), c);
		return NULL;
	}
	return lValBytecodeOp(arr->data[i]);
}

static lVal *lnfBytecodeArrLength(lClosure *c, lVal *v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));
	return lValInt(arr->dataEnd - arr->data);
}

static lVal *lnfBytecodeEval(lClosure *c, lVal *v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));
	lClosure *env = requireClosure(c, lCadr(v));
	return lBytecodeEval(env, arr);
}

void lOperationsBytecode(lClosure *c){
	lAddNativeFuncPure(c,"int->bytecode-op", "[a]", "Turns an integer into a bytecode operation with the same value", lnfIntBytecodeOp);
	lAddNativeFuncPure(c,"bytecode-op->int", "[a]", "Turns a bytecode operation into an integer of the same value", lnfBytecodeOpInt);
	lAddNativeFunc(c,"arr->bytecode-arr",    "[a]", "Turns an array of bytecode operations into a bytecode array", lnfArrBytecodeArr);
	lAddNativeFunc(c,"bytecode-arr->arr",    "[a]", "Turns an bytecode array into an array of bytecode operations", lnfBytecodeArrArr);
	lAddNativeFunc(c,"bytecode-literals",    "[a]", "Return the literal section of a BCA", lnfBytecodeLiterals);

	lAddNativeFunc(c,"bytecode-array/ref",   "[a i]", "Return the bytecode-op in A at position I", lnfBytecodeArrRef);
	lAddNativeFunc(c,"bytecode-array/length","[a]", "Return the length of the bytecode-array A", lnfBytecodeArrLength);

	lAddNativeFunc(c,"bytecode-eval*","[bc-arr env]", "Evaluate BC-ARR in ENV", lnfBytecodeEval);
}
