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
	lBytecodeArray *bca = lBytecodeArrayAlloc(len);
	ret->vBytecodeArr = bca;

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

static lVal *lnfBytecodeEval(lClosure *c, lVal *v){
	lBytecodeArray *arr = requireBytecodeArray(c, lCar(v));
	lVal *env = lCadr(v);
	lClosure *bcc = c;
	const bool trace = castToBool(lCaddr(v));
	if(env){
		requireEnvironment(c, env);
		bcc = env->vClosure;
	}
	return lBytecodeEval(bcc, arr, trace);
}

static lVal *lnfValIndex(lClosure *c, lVal *v){
	(void)c;
	return lValInt(lValIndex(lCar(v)));
}

static lVal *lnfIndexVal(lClosure *c, lVal *v){
	return lIndexVal(requireInt(c, lCar(v)));
}

static lVal *lnfSymIndex(lClosure *c, lVal *v){
	return lValInt(lSymIndex(requireSymbolic(c, lCar(v))));
}

static lVal *lnfIndexSym(lClosure *c, lVal *v){
	return lValSymS(lIndexSym(requireInt(c, lCar(v))));
}

void lOperationsBytecode(lClosure *c){
	lAddNativeFunc(c,"int->bytecode-op",  "[a]", "Turns an integer into a bytecode operation with the same value", lnfIntBytecodeOp);
	lAddNativeFunc(c,"bytecode-op->int",  "[a]", "Turns a bytecode operation into an integer of the same value", lnfBytecodeOpInt);
	lAddNativeFunc(c,"arr->bytecode-arr", "[a]", "Turns an array of bytecode operations into a bytecode array", lnfArrBytecodeArr);
	lAddNativeFunc(c,"bytecode-arr->arr", "[a]", "Turns an bytecode array into an array of bytecode operations", lnfBytecodeArrArr);

	lAddNativeFunc(c,"val->index", "[v]", "Return an index value pointing to V", lnfValIndex);
	lAddNativeFunc(c,"index->val", "[i]", "Return the value at index position I", lnfIndexVal);
	lAddNativeFunc(c,"sym->index", "[v]", "Return an index value pointing to symbol V", lnfSymIndex);
	lAddNativeFunc(c,"index->sym", "[i]", "Return the symbol at index position I", lnfIndexSym);

	lAddNativeFunc(c,"bytecode-eval",    "[bc environment trace]", "Evaluate a bytecode array and return the result", lnfBytecodeEval);
}
