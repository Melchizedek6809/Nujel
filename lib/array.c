/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <stdlib.h>
#include <string.h>

static lVal lnfArrLength(lVal a){
	lVal car = requireArray(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lArray *arr = car.vArray;
	return lValInt(arr->length);
}

static lVal lnfArrLengthSet(lVal a, lVal b){
	lVal car = requireArray(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lArray *arr = car.vArray;
	lVal lenVal = requireNaturalInt(b);
	if(unlikely(lenVal.type == ltException)){
		return lenVal;
	}
	const int length = lenVal.vInt;
	lVal *newData = realloc(arr->data,length * sizeof(lVal));
	if (unlikely(newData == NULL)) {
		free(newData);
		return lValException(lSymOOM, "(array/allocate) couldn't allocate its array", b);
	}
	arr->data = newData;
	if(length > arr->length){
		memset(&arr->data[arr->length], 0, (((size_t)length) - arr->length) * sizeof(lVal *));
	}
	arr->length = length;
	return car;
}

static lVal lnfArrAllocate(lVal a){
	lVal lenV = requireNaturalInt(a);
	if(unlikely(lenV.type == ltException)){
		return lenV;
	}
	const int len = lenV.vInt;
	lVal r = lValAlloc(ltArray, lArrayAlloc(len));
	if(unlikely(len && (r.vArray->data == NULL))){
		return lValException(lSymOOM, "(array/allocate) couldn't allocate its array", a);
	}
	return r;
}

/* Return the length of the list V */
static int lListLength(lVal v){
	int i = 0;
	for(lVal n = v;(n.type == ltPair) && (n.vList->car.type != ltNil); n = n.vList->cdr){
		i++;
	}
	return i;
}

lVal lnfArrNew(lVal v){
	int length = lListLength(v);
	lVal r = lValAlloc(ltArray, lArrayAlloc(length));
	int key = 0;
	for(lVal n = v; n.type == ltPair; n = n.vList->cdr){
		if(unlikely(key >= length)){break;}
		r.vArray->data[key++] = n.vList->car;
	}
	return r;
}

void lOperationsArray(lClosure *c){
	lAddNativeFuncR (c,"array/new",      "args",                "Create a new array from ...ARGS",          lnfArrNew, 0);
	lAddNativeFuncV (c,"array/allocate", "(size)",              "Allocate a new array of SIZE",             lnfArrAllocate, 0);
	lAddNativeFuncV (c,"array/length",   "(array)",             "Return length of ARRAY",                   lnfArrLength, 0);
	lAddNativeFuncVV(c,"array/length!",  "(array size)",        "Set a new LENGTH for ARRAY",               lnfArrLengthSet, 0);
}
