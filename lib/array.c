/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <stdlib.h>
#include <string.h>

static lVal lnmArrayLength(lVal self){
	return lValInt(self.vArray->length);
}

static lVal lnmArrayLengthSet(lVal self, lVal newLength){
	lVal e = requireNaturalInt(newLength);
	if(unlikely(e.type == ltException)){
		return e;
	}
	const size_t length = e.vInt;
	lArray *arr = self.vArray;

	lVal *newData = realloc(arr->data,length * sizeof(lVal));
	if (unlikely(newData == NULL)) {
		free(newData);
		return lValException(lSymOOM, "(:length Array) couldn't allocate its array", self);
	}
	arr->data = newData;
	if(length > (size_t)arr->length){
		memset(&arr->data[arr->length], 0, (length - arr->length) * sizeof(lVal));
	}
	arr->length = length;
	return self;
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
	lClass *Array = &lClassList[ltArray];
	lAddNativeFuncR (c,"array/new",      "args",                "Create a new array from ...ARGS",          lnfArrNew, 0);
	lAddNativeFuncV (c,"array/allocate", "(size)",              "Allocate a new array of SIZE",             lnfArrAllocate, 0);
	lAddNativeMethodV(Array, lSymS("length"),  "(self)", lnmArrayLength, 0);
	lAddNativeMethodVV(Array, lSymS("length!"), "(self new-size)", lnmArrayLengthSet, 0);
}
