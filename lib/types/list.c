/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "list.h"
#include "val.h"

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

lVal *lCar(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.car : NULL;
}

lVal *lCdr(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.cdr : NULL;
}

lVal *lCaar(lVal *v){
	return lCar(lCar(v));
}

lVal *lCadr(lVal *v){
	return lCar(lCdr(v));
}

lVal *lCdar(lVal *v){
	return lCdr(lCar(v));
}

lVal *lCddr(lVal *v){
	return lCdr(lCdr(v));
}

lVal *lCadar(lVal *v){
	return lCar(lCdr(lCar(v)));
}

lVal *lCaddr(lVal *v){
	return lCar(lCdr(lCdr(v)));
}

lVal *lCdddr(lVal *v){
	return lCdr(lCdr(lCdr(v)));
}

int lListLength(lVal *v){
	int i = 0;
	for(lVal *n = v;(n != NULL) && (lCar(n) != NULL); n = lCdr(n)){i++;}
	return i;
}
