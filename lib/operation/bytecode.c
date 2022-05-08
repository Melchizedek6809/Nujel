/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../exception.h"
#include "../collection/list.h"
#include "../type/closure.h"
#include "../type-system.h"
#include "../vm/eval.h"

#include <stdlib.h>

static lVal *lValBytecodeOp(lBytecodeOp v){
	lVal *ret = lValAlloc(ltBytecodeOp);
	ret->vBytecodeOp = v;
	return ret;
}

static lVal *lnfIntBytecodeOp(lClosure *c, lVal *v){
	const i64 val = castToInt(lCar(v), -1);
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

	lVal *ret = lValAlloc(ltBytecodeArr);
	ret->vBytecodeArr.data = malloc(len * sizeof(lBytecodeOp));
	ret->vBytecodeArr.dataEnd = &ret->vBytecodeArr.data[len];

	for(int i=0;i<len;i++){
		ret->vBytecodeArr.data[i] = requireBytecodeOp(c, arr->data[i]);
	}

	return ret;
}

static lVal *lnfBytecodeArrArr(lClosure *c, lVal *v){
	lBytecodeArray arr = requireBytecodeArray(c, lCar(v));
	const int len = arr.dataEnd - arr.data;

	lVal *ret = RVP(lValAlloc(ltArray));
	ret->vArray = lArrayAlloc();
	ret->vArray->length = len;
	ret->vArray->data = malloc(len * sizeof(*ret->vArray->data));

	for(int i=0;i<len;i++){
		ret->vArray->data[i] = lValBytecodeOp(arr.data[i]);
	}

	return ret;
}

static lVal *lnfBytecodeEval(lClosure *c, lVal *v){
	lBytecodeArray arr = requireBytecodeArray(c, lCar(v));
	lVal *args = lCadr(v);
	lVal *env = lCaddr(v);
	lClosure *bcc = c;
	const bool trace = castToBool(lCadddr(v));
	if(env){
		if(env->type != ltObject){
			lExceptionThrowValClo("type-error", "Environments have to be of type :object", lCaddr(v), c);
		}
		bcc = env->vClosure;
	}
	return lBytecodeEval(bcc, args, &arr, trace);
}

void lOperationsBytecode(lClosure *c){
	lAddNativeFunc(c,"int->bytecode-op",  "[a]", "Turns an integer into a bytecode operation with the same value", lnfIntBytecodeOp);
	lAddNativeFunc(c,"bytecode-op->int",  "[a]", "Turns a bytecode operation into an integer of the same value", lnfBytecodeOpInt);
	lAddNativeFunc(c,"arr->bytecode-arr", "[a]", "Turns an array of bytecode operations into a bytecode array", lnfArrBytecodeArr);
	lAddNativeFunc(c,"bytecode-arr->arr", "[a]", "Turns an bytecode array into an array of bytecode operations", lnfBytecodeArrArr);
	lAddNativeFunc(c,"bytecode-eval",     "[bc args environment trace]", "Evaluate a bytecode array and return the result", lnfBytecodeEval);
}
