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

lVal *lnfvArrRef;

lVal *lnfArrLength(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	if((arr == NULL) || (arr->type != ltArray)){return lValInt(0);}
	return lValInt(arr->vArray->length);
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

lVal *lnfArrSet(lClosure *c, lVal *v){
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

lVal *lnfArrNew(lClosure *c, lVal *v){
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

lVal *lnfArr(lClosure *c, lVal *v){
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

lVal *lnfArrPred(lClosure *c, lVal *v){
	(void)c;
	lVal *arr = lCar(v);
	return lValBool(!((arr == NULL) || (arr->type != ltArray)));
}

void lOperationsArray(lClosure *c){
	lnfvArrRef = lAddNativeFunc(c,"arr-ref",    "[array index]",  "Return value of ARRAY at position INDEX",     lnfArrRef);
	lAddNativeFunc(c,"arr-length", "[array]",    "Return length of ARRAY",                          lnfArrLength);
	lAddNativeFunc(c,"arr-set!",   "[array index &...values]","Set ARRAY at INDEX to &...VALUES",   lnfArrSet);
	lAddNativeFunc(c,"arr-new",    "[size]",     "Allocate a new array of SIZE",                    lnfArrNew);
	lAddNativeFunc(c,"arr",        "[...args]",  "Create a new array from ...ARGS",                 lnfArr);
	lAddNativeFunc(c,"array? arr?","[val]",     "Return #t if VAL is an array",                     lnfArrPred);
}
