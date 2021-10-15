/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "array.h"
#include "../datatypes/array.h"
#include "../datatypes/list.h"
#include "../datatypes/native-function.h"
#include "../datatypes/val.h"
#include "../casting.h"

#ifndef COSMOPOLITAN_H_
	#include <stdlib.h>
	#include <string.h>
#endif

lVal *lnfArrLength(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){return lValInt(0);}
	return lValInt(arr->vArray->length);
}

lVal *lnfArrRef(lClosure *c, lVal *v){
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){return NULL;}
	v = lCdr(v);
	lVal *t = lCar(v);
	if(t == NULL){return arr;}
	if((t->type != ltInt) && (t->type != ltFloat)){return NULL;}
	const lVal *lkey = lnfInt(c,t);
	if(lkey == NULL){return NULL;}
	const int key = lkey->vInt;
	if((key < 0) || (key >= arr->vArray->length)){return NULL;}
	return arr->vArray->data[key];
}

lVal *lnfArrSet(lClosure *c, lVal *v){
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray) || (v == NULL)){return NULL;}
	v = lCdr(v);
	lVal *t = lCar(v);
	if(t == NULL){return NULL;}
	if((t->type != ltInt) && (t->type != ltFloat)){return NULL;}
	const lVal *lkey = lnfInt(c,t);
	if(lkey == NULL){return NULL;}
	int key = lkey->vInt;
	if((key < 0) || (key >= arr->vArray->length)){return NULL;}
	v = lCdr(v);
	forEach(cur,v){
		lVal *cv = lCar(cur);
		arr->vArray->data[key++] = cv;
		if(key >= arr->vArray->length){return NULL;}
	}
	return arr->vArray->data[key];
}


lVal *lnfArrNew(lClosure *c, lVal *v){
	lVal *t = lnfInt(c,v);
	if((t == NULL) || (t->type != ltInt)){return NULL;}
	lVal *r = lValAlloc();
	r->type = ltArray;
	r->vArray = lArrayAlloc();
	r->vArray->length = t->vInt;
	r->vArray->data = calloc(t->vInt,sizeof(*r->vArray->data));
	if(r->vArray->data == NULL){
		lArrayFree(r->vArray);
		lValFree(r);
		return NULL;
	}
	return r;
}

lVal *lnfArr(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}
	int length = lListLength(v);
	lVal *r = lValAlloc();
	r->type = ltArray;
	lArray *arr = lArrayAlloc();
	if(arr == NULL){return NULL;}
	r->vArray = arr;
	arr->length = length;
	arr->data = calloc(length,sizeof(*arr->data));
	if(arr->data == NULL){
		arr->length = 0;
		lArrayFree(arr);
		lValFree(r);
		return NULL;
	}
	int key = 0;
	forEach(cur, v){
		arr->data[key++] = lCar(cur);
	}
	return r;
}

lVal *lnfArrPred(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){return lValBool(false);}
	return lValBool(true);
}

void lOperationsArray(lClosure *c){
	lAddNativeFunc(c,"arr-length", "[array]",    "Return length of ARRAY",                          lnfArrLength);
	lAddNativeFunc(c,"arr-ref",    "[array index]",  "Return value of ARRAY at position INDEX",     lnfArrRef);
	lAddNativeFunc(c,"arr-set!",   "[array index &...values]","Set ARRAY at INDEX to &...VALUES",   lnfArrSet);
	lAddNativeFunc(c,"arr-new",    "[size]",     "Allocate a new array of SIZE",                    lnfArrNew);
	lAddNativeFunc(c,"arr",        "[...args]",  "Create a new array from ...ARGS",                 lnfArr);
	lAddNativeFunc(c,"array? arr?","[val]",     "Return #t if VAL is an array",                     lnfArrPred);
}
