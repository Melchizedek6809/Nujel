/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "list.h"
#include "../exception.h"
#include "../nujel.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/val.h"

static lVal *lnfCar(lClosure *c, lVal *v){
	(void)c;
	return lCaar(v);
}

static lVal *lnfCdr(lClosure *c, lVal *v){
	(void)c;
	return lCdar(v);
}

static lVal *lnfCons(lClosure *c, lVal *v){
	(void)c;
	if(lCddr(v) != NULL){
		lExceptionThrowValClo(":too-many-args","Cons should only be called with 2 arguments!", v, c);
	}
	return lCons(lCar(v),lCadr(v));
}

void lOperationsList(lClosure *c){
	lAddNativeFunc(c,"car",  "[list]",     "Returs the head of LIST",          lnfCar);
	lAddNativeFunc(c,"cdr",  "[list]",     "Return the rest of LIST",          lnfCdr);
	lAddNativeFunc(c,"cons", "[car cdr]",  "Return a new pair of CAR and CDR", lnfCons);
}
