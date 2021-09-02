/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "boolean.h"
#include "casting.h"

lVal *lnfNot(lClosure *c, lVal *v){
	lVal *a = lnfBool(c,v);
	return lValBool(a == NULL ? true : !a->vBool);
}

lVal *lnfAnd(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(true);}
	lVal *t = lnfBool(c,v);
	if((t == NULL) || (!t->vBool)){return lValBool(false);}
	return lnfAnd(c,lCdr(v));
}

lVal *lnfOr(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lnfBool(c,v);
	if((t != NULL) && t->vBool){return lValBool(true);}
	return lnfOr(c,lCdr(v));
}

void lAddBooleanFuncs(lClosure *c){
	lAddNativeFunc(c,"and &&", "[...args]", "#t if all ARGS evaluate to true",            lnfAnd);
	lAddNativeFunc(c,"or ||" , "[...args]", "#t if one member of ARGS evaluates to true", lnfOr);
	lAddNativeFunc(c,"not !",  "[val]",     "#t if VAL is #f, #f if VAL is #t",           lnfNot);
}
