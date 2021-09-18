/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "list.h"
#include "../datatypes/list.h"
#include "../datatypes/native-function.h"
#include "../datatypes/val.h"
#include "../nujel.h"

static lVal *lnfCar(lClosure *c, lVal *v){
	return lCar(lEval(c,lCar(v)));
}

static lVal *lnfCdr(lClosure *c, lVal *v){
	return lCdr(lEval(c,lCar(v)));
}

static lVal *lnfCons(lClosure *c, lVal *v){
	return lCons(lEval(c,lCar(v)),lEval(c,lCadr(v)));
}
static lVal *lnfSetCar(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	lVal *car = NULL;
	if((v != NULL) && (v->type == ltPair) && (lCdr(v) != NULL)){car = lEval(c,lCadr(v));}
	t->vList.car = car;
	return t;
}
static lVal *lnfSetCdr(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	lVal *cdr = NULL;
	if((v != NULL) && (v->type == ltPair) && (lCdr(v) != NULL)){cdr = lEval(c,lCadr(v));}
	t->vList.cdr = cdr;
	return t;
}

void lOperationsList(lClosure *c){
	lAddNativeFunc(c,"car",     "[list]",     "Returs the head of LIST",                          lnfCar);
	lAddNativeFunc(c,"cdr",     "[list]",     "Return the rest of LIST",                          lnfCdr);
	lAddNativeFunc(c,"cons",    "[car cdr]",  "Return a new pair of CAR and CDR",                lnfCons);
	lAddNativeFunc(c,"set-car!","[list car]", "Set the CAR of LIST",                           lnfSetCar);
	lAddNativeFunc(c,"set-cdr!","[list cdr]", "Set the CDR of LIST",                           lnfSetCdr);
}
