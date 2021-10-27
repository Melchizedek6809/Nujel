/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "list.h"
#include "../type/val.h"

lVal *lCons(lVal *car, lVal *cdr){
	lVal *v = lValAlloc();
	if(v == NULL){return NULL;}
	v->type = ltPair;
	v->vList.car = car;
	v->vList.cdr = cdr;
	return v;
}

lVal *lLastCar(lVal *v){
	forEach(a,v){
		if(lCdr(a) == NULL){return lCar(a);}
	}
	return NULL;
}

int lListLength(lVal *v){
	int i = 0;
	for(lVal *n = v;(n != NULL) && (lCar(n) != NULL); n = lCdr(n)){i++;}
	return i;
}
