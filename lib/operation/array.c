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

static lVal *lnfArrLength(lClosure *c, lVal *v){
	return lValInt(requireArray(c, lCar(v))->length);
}

static lVal *lnfArrLengthSet(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	lArray *arr = requireArray(c, car);
	const int length = requireNaturalInt(c, lCadr(v));
	arr->data = realloc(arr->data,length * sizeof(lVal *));
	if(length > arr->length){
		memset(&arr->data[arr->length], 0, (length - arr->length) * sizeof(lVal *));
	}
	arr->length = length;
	return car;
}

static lVal *lnfArrSet(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	lArray *arr = requireArray(c, car);
	const int key = requireInt(c, lCadr(v));
	if((key < 0) || (key >= arr->length)){
		lExceptionThrowValClo("out-of-bounds","[array/set!] index provided is out of bounds", v, c);
		return NULL;
	}
	const lVal *vt = lCddr(v);
	if((vt == NULL) || (vt->type != ltPair)){
		lExceptionThrowValClo("type-mismatch","[array/set!] needs a third argument", v, c);
		return NULL;
	}
	arr->data[key] = vt->vList.car;
	return car;
}

static lVal *lnfArrAllocate(lClosure *c, lVal *v){
	(void)c;
	const int len = requireNaturalInt(c, lCar(v));
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

static lVal *lnfArrRef(lClosure *c, lVal *v){
	lArray *arr = requireArray(c,  lCar(v));
	const int key = requireInt(c, lCadr(v));
	if((key < 0) || (key >= arr->length)){
		lExceptionThrowValClo("out-of-bounds","[array/ref] index provided is out of bounds", v, c);
		return NULL;
	}
	return arr->data[key];
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
	for(lVal *n = v; n && n->type == ltPair; n = n->vList.cdr){
		if(key >= length){break;}
		r->vArray->data[key++] = n->vList.car;
	}
	return r;
}

void lOperationsArray(lClosure *c){
	lAddNativeFunc(c,"array/new",      "args",                "Create a new array from ...ARGS",          lnfArrNew);
	lAddNativeFunc(c,"array/ref",      "[array index]",       "Return value of ARRAY at position INDEX",  lnfArrRef);
	lAddNativeFunc(c,"array/length",   "[array]",             "Return length of ARRAY",                   lnfArrLength);
	lAddNativeFunc(c,"array/length!",  "[array size]",        "Set a new LENGTH for ARRAY",               lnfArrLengthSet);
	lAddNativeFunc(c,"array/set!",     "[array index value]", "Set ARRAY at INDEX to &...VALUES",         lnfArrSet);
	lAddNativeFunc(c,"array/allocate", "[size]",              "Allocate a new array of SIZE",             lnfArrAllocate);
}
