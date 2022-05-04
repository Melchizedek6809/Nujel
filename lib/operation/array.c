/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../exception.h"
#include "../misc/pf.h"
#include "../collection/list.h"
#include "../type/native-function.h"

#ifdef __WATCOMC__
#include <malloc.h>
#endif

#include <stdlib.h>
#include <string.h>

lVal *lnfvArrRef;

static lVal *lnfArrLength(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){
		lExceptionThrowValClo("type-mismatch","[array/length] expects an array as its first and only argument", v, c);
		return NULL;
	}
	return lValInt(arr->vArray->length);
}

static lVal *lnfArrLengthSet(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){
		lExceptionThrowValClo("type-mismatch","[array/length!] expects an array as its first argument", v, c);
		return NULL;
	}
	const int length =castToInt(lCadr(v),-1);
	if(length >= 0){
		arr->vArray->data = realloc(arr->vArray->data,length * sizeof(lVal *));
		if(length > arr->vArray->length){
			memset(&arr->vArray->data[arr->vArray->length], 0, (length - arr->vArray->length) * sizeof(lVal *));
		}
		arr->vArray->length = length;
	}
	return arr;
}

static lVal *lnfArrSet(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){
		lExceptionThrowValClo("type-mismatch","[array/set!] expects an array as its first argument", v, c);
		return NULL;
	}
	v = lCdr(v);
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltInt)){
		lExceptionThrowValClo("type-mismatch","[array/set!] expects its second argument to be an integer", v, c);
		return NULL;
	}
	const int key = t->vInt;
	if((key < 0) || (key >= arr->vArray->length)){
		lExceptionThrowValClo("out-of-bounds","[array/set!] index provided is out of bounds", v, c);
		return NULL;
	}
	const lVal *vt = lCdr(v);
	if((vt == NULL) || (vt->type != ltPair)){
		lExceptionThrowValClo("type-mismatch","[array/set!] expects a third argument", v, c);
		return NULL;
	}
	arr->vArray->data[key] = lCadr(v);
	return arr;
}

static lVal *lnfArrAllocate(lClosure *c, lVal *v){
	(void)c;
	const int len = castToInt(lCar(v),-1);
	if(len < 0){
		lExceptionThrowValClo("invalid-argument","[array/allocate] expects an integer indicating the size which has to be at least 0 or more", v, c);
		return NULL;
	}
	lVal *r = lRootsValPush(lValAlloc(ltArray));
	r->vArray = lArrayAlloc();
	r->vArray->length = len;
	r->vArray->data = calloc(len, sizeof(*r->vArray->data));
	if(len && (r->vArray->data == NULL)){
		lExceptionThrowValClo("out-of-memory","[array/allocate] couldn't allocate its array", v, c);
		return NULL;
	}
	return r;
}

lVal *lnfArrNew(lClosure *c, lVal *v){
	(void)c;
	int length = v ? lListLength(v) : 0;
	lVal *r = lRootsValPush(lValAlloc(ltArray));
	r->vArray = lArrayAlloc();
	r->vArray->length = length;
	r->vArray->data = calloc(length, sizeof(*r->vArray->data));
	if(length && (r->vArray->data == NULL)){
		lExceptionThrowValClo("out-of-memory","[array/new] couldn't allocate its array", lValInt(length), c);
		return NULL;
	}
	int key = 0;
	forEach(cur, v){
		r->vArray->data[key++] = lCar(cur);
	}
	return r;
}

lVal *lnfArrRef(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){
		lExceptionThrowValClo("type-mismatch","[array/ref] expects an array as its first argument", v, c);
		return NULL;
	}
	lVal *t = lCadr(v);
	if((t == NULL) || (t->type != ltInt)){
		lExceptionThrowValClo("type-mismatch","[array/ref] expects its second argument to be an integer", v, c);
		return NULL;
	}
	const int key = t->vInt;
	if((key < 0) || (key >= arr->vArray->length)){
		lExceptionThrowValClo("out-of-bounds","[array/ref] index provided is out of bounds", v, c);
		return NULL;
	}
	return arr->vArray->data[key];
}

void lOperationsArray(lClosure *c){
	             lAddNativeFunc(c,"array/new",      "args",                "Create a new array from ...ARGS",                lnfArrNew);
	lnfvArrRef = lAddNativeFunc(c,"array/ref",      "[array index]",       "Return value of ARRAY at position INDEX",     lnfArrRef);
	             lAddNativeFunc(c,"array/length",   "[array]",             "Return length of ARRAY",                      lnfArrLength);
	             lAddNativeFunc(c,"array/length!",  "[array size]",        "Set a new LENGTH for ARRAY",                  lnfArrLengthSet);
	             lAddNativeFunc(c,"array/set!",     "[array index value]", "Set ARRAY at INDEX to &...VALUES",  lnfArrSet);
	             lAddNativeFunc(c,"array/allocate", "[size]",              "Allocate a new array of SIZE",                   lnfArrAllocate);
}
