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

lVal lValBytecodeArray(const lBytecodeOp *ops, int opsLength, lArray *literals, lClosure *errorClosure){
	if(!lBytecodeArrayCheckIfValid(ops, opsLength, literals)){
		lExceptionThrowValClo("invalid-bc-array", "Invalid bytecode array", NIL, errorClosure);
		return NIL;
	}
	lVal ret = lValAlloc(ltBytecodeArr, lBytecodeArrayAlloc(opsLength));
	ret.vBytecodeArr->literals = literals;
	ret.vBytecodeArr->literals->flags |= ARRAY_IMMUTABLE;
	memcpy(ret.vBytecodeArr->data, ops, opsLength);
	return ret;
}

static lVal lnfArrBytecodeArr(lClosure *c, lVal v){
	lArray *arr = requireArray(c, lCar(v));
	const int len = arr->length;
	lArray *literals = requireArray(c, lCadr(v));
	lBytecodeOp *ops = malloc(sizeof(lBytecodeOp) * len);
	for(int i=0;i<len;i++){
		ops[i] = requireInt(c, arr->data[i]);
	}
	if(!lBytecodeArrayCheckIfValid(ops, len, literals)){
		lExceptionThrowValClo("invalid-bc-array", "The bytecodes and literal array are invalid", v, c);
		return NIL;
	}
	lVal ret = lValBytecodeArray(ops,len,literals,c);
	free(ops);
	return ret;
}

static lVal lnfBytecodeArrArr(lClosure *c, lVal v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));
	const int len = arr->dataEnd - arr->data;

	lVal ret = lValAlloc(ltArray, lArrayAlloc(len));
	for(int i=0;i<len;i++){
		ret.vArray->data[i] = lValInt(arr->data[i]);
	}
	return ret;
}

static lVal lnfBytecodeLiterals(lClosure *c, lVal v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));
	if(unlikely(arr->literals == NULL)){return NIL;}
	return lValAlloc(ltArray, arr->literals);
}

static lVal lnfBytecodeArrLength(lClosure *c, lVal v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));
	return lValInt(arr->dataEnd - arr->data);
}

void lOperationsBytecode(lClosure *c){
	lAddNativeFunc(c,"arr->bytecode-arr",    "(a)", "Turns an array of bytecode operations into a bytecode array", lnfArrBytecodeArr);
	lAddNativeFunc(c,"bytecode-arr->arr",    "(a)", "Turns an bytecode array into an array of bytecode operations", lnfBytecodeArrArr);
	lAddNativeFunc(c,"bytecode-literals",    "(a)", "Return the literal section of a BCA", lnfBytecodeLiterals);

	lAddNativeFunc(c,"bytecode-array/length","(a)", "Return the length of the bytecode-array A", lnfBytecodeArrLength);
}
