/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "array.h"
#include "../display.h"
#include "../allocation/array.h"
#include "../allocation/roots.h"
#include "../allocation/val.h"
#include "../type-system.h"
#include "../collection/list.h"
#include "../type/native-function.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

lVal *lnfvArrRef;

static lVal *lnfArrLength(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){return lValInt(0);}
	return lValInt(arr->vArray->length);
}

static lVal *lnfArrLengthSet(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){return NULL;}
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
	if((arr == NULL) || (arr->type != ltArray) || (v == NULL)){return NULL;}
	v = lCdr(v);
	lVal *t = lCar(v);
	if(t == NULL){return NULL;}
	if((t->type != ltInt) && (t->type != ltFloat)){return NULL;}
	int key = castToInt(t,0);
	if((key < 0) || (key >= arr->vArray->length)){return NULL;}
	v = lCdr(v);
	forEach(cur,v){
		lVal *cv = lCar(cur);
		arr->vArray->data[key++] = cv;
		if(key >= arr->vArray->length){return NULL;}
	}
	return arr->vArray->data[key];
}

static lVal *lnfArrNew(lClosure *c, lVal *v){
	(void)c;
	const int len = castToInt(lCar(v),-1);
	if(len < 0){return NULL;}
	lVal *r = lRootsValPush(lValAlloc());
	r->type = ltArray;
	r->vArray = lArrayAlloc();
	r->vArray->length = len;
	r->vArray->data = calloc(len,sizeof(*r->vArray->data));
	if(r->vArray->data == NULL){
		lPrintError("lnfArrNew OOM\n");
		return NULL;
	}
	return r;
}

static lVal *lnfArr(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	int length = lListLength(v);
	lVal *r = lRootsValPush(lValAlloc());
	r->type = ltArray;
	r->vArray = lArrayAlloc();
	r->vArray->length = length;
	r->vArray->data = calloc(length,sizeof(*r->vArray->data));
	if(r->vArray->data == NULL){
		lPrintError("lnfArr OOM\n");
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
	if((arr == NULL) || (arr->type != ltArray)){return NULL;}
	lVal *t = lCadr(v);
	if(t == NULL){return arr;}
	if((t->type != ltInt) && (t->type != ltFloat)){return NULL;}
	const int key = castToInt(t,-1);
	if((key < 0) || (key >= arr->vArray->length)){return NULL;}
	return arr->vArray->data[key];
}

void lOperationsArray(lClosure *c){
	lnfvArrRef = lAddNativeFunc(c,"array/ref",      "[array index]", "Return value of ARRAY at position INDEX",     lnfArrRef);
	             lAddNativeFunc(c,"array/length",   "[array]",       "Return length of ARRAY",                      lnfArrLength);
	             lAddNativeFunc(c,"array/length!",  "[array size]",  "Set a new LENGTH for ARRAY",                  lnfArrLengthSet);
	             lAddNativeFunc(c,"array/set!",     "[array index &...values]","Set ARRAY at INDEX to &...VALUES",  lnfArrSet);
	             lAddNativeFunc(c,"array/allocate", "[size]",     "Allocate a new array of SIZE",                   lnfArrNew);
	             lAddNativeFunc(c,"array/new",      "[...args]",  "Create a new array from ...ARGS",                lnfArr);
}
