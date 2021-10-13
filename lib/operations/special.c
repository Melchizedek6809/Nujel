/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "special.h"
#include "../casting.h"
#include "../datatypes/native-function.h"
#include "../datatypes/list.h"
#include "../datatypes/val.h"

static lVal *lnfAnd(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lEval(c,lCar(v));
	if(lBool(t)){
		lVal *cdr = lCdr(v);
		return cdr == NULL ? t : lnfAnd(c,cdr);
	}
	return lValBool(false);
}

static lVal *lnfOr(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lEval(c,lCar(v));
	return lBool(t) ? t : lnfOr(c,lCdr(v));
}

static lVal *lnfCond(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	if(t == NULL){return NULL;}
	return lBool(lEval(c,lCar(t)))
	       ? lnfBegin(c,lCdr(t))
	       : lnfCond(c,lCdr(v));
}

static lVal *lnfWhen(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	if(!lBool(lEval(c,lCar(v)))){return NULL;}
	return lnfBegin(c,lCdr(v));
}

static lVal *lnfUnless(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	if(lBool(lEval(c,lCar(v)))){return NULL;}
	return lnfBegin(c,lCdr(v));
}

static lVal *lnfIf(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	const bool pred = lBool(lEval(c,lCar(v)));
	return lEval(c,pred ? lCadr(v) : lCaddr(v));
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	(void)c;
	return lCar(v);
}

lVal *lnfBegin(lClosure *c, lVal *v){
	lVal *ret = NULL;
	forEach(n,v){
		ret = lEval(c,lCar(n));
	}
	return ret;
}

void lOperationsSpecial(lClosure *c){
	lAddSpecialForm(c,"if",      "[pred? then ...else]","Evalute then if pred? is #t, otherwise evaluates ...else", lnfIf);
	lAddSpecialForm(c,"cond",    "[...c]",              "Contain at least 1 cond block of form (pred? ...body) and evaluates and returns the first where pred? is #t", lnfCond);
	lAddSpecialForm(c,"when",    "[condition ...body]", "Evaluates BODY if CONDITION is #t", lnfWhen);
	lAddSpecialForm(c,"unless",  "[condition ...body]", "Evaluates BODY if CONDITION is #f", lnfUnless);
	lAddSpecialForm(c,"and &&",  "[...args]",           "#t if all ARGS evaluate to true", lnfAnd);
	lAddSpecialForm(c,"or ||" ,  "[...args]",           "#t if one member of ARGS evaluates to true", lnfOr);
	lAddSpecialForm(c,"do begin","[...body]",           "Evaluate ...body in order and returns the last result", lnfBegin);
	lAddSpecialForm(c,"quote",   "[v]",                 "Return v as is without evaluating",         lnfQuote);
}
