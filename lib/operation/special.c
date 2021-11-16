/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "special.h"
#include "../api.h"
#include "../exception.h"
#include "../type-system.h"
#include "../allocation/roots.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/val.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

lVal *lnfvDo;
lVal *lnfvQuote;

static lVal *lnfAnd(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lEval(c,lCar(v));
	if(castToBool(t)){
		lVal *cdr = lCdr(v);
		return cdr == NULL ? t : lnfAnd(c,cdr);
	}
	return lValBool(false);
}

static lVal *lnfOr(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lEval(c,lCar(v));
	return castToBool(t) ? t : lnfOr(c,lCdr(v));
}

static lVal *lnfCond(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	if(t == NULL){return NULL;}
	return castToBool(lEval(c,lCar(t)))
	       ? lnfDo(c,lCdr(t))
	       : lnfCond(c,lCdr(v));
}

static lVal *lnfWhen(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	if(!castToBool(lEval(c,lCar(v)))){return NULL;}
	return lnfDo(c,lCdr(v));
}

static lVal *lnfUnless(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	if(castToBool(lEval(c,lCar(v)))){return NULL;}
	return lnfDo(c,lCdr(v));
}

static lVal *lnfIf(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	const bool pred = castToBool(lEval(c,lCar(v)));
	return lEval(c,pred ? lCadr(v) : lCaddr(v));
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	(void)c;
	return lCar(v);
}

lVal *lnfDo(lClosure *c, lVal *v){
	lVal *ret = NULL;
	const int SP = lRootsGet();
	forEach(n,v){
		ret = lEval(c,lCar(n));
		lRootsRet(SP);
	}
	return ret;
}

lVal *lnfWhile(lClosure *c, lVal *v){
	lVal *cond = lCar(v);
	lVal *body = lCdr(v);
	lVal *ret  = NULL;
	const int SP = lRootsGet();
	while(castToBool(lEval(c,cond))){
		ret = lnfDo(c,body);
		lRootsRet(SP);
	}
	return ret;
}

lVal *lnfTry(lClosure *c, lVal *v){
	const int SPOuter    = lRootsGet();
	lVal *volatile catch = lRootsValPush(lEval(c,lCar(v)));
	lVal *volatile body  = lCdr(v);

	lVal *ret = lTry(c,catch,body);

	lRootsRet(SPOuter);

	return ret;
}

lVal *lnfThrow(lClosure *c, lVal *v){
	(void)c;
	lExceptionThrowRaw(lCar(v));
	return NULL;
}

void lOperationsSpecial(lClosure *c){
	lnfvDo    = lAddSpecialForm(c,"do",    "[...body]", "Evaluate ...body in order and returns the last result", lnfDo);
	lnfvQuote = lAddSpecialForm(c,"quote", "[v]",       "Return v as is without evaluating", lnfQuote);
	lAddSpecialForm(c,"if",      "[cond then else]","Evalute then if pred? is #t, otherwise evaluates ...else", lnfIf);
	lAddSpecialForm(c,"cond",    "[...c]",          "Contain at least 1 cond block of form (pred? ...body) and evaluates and returns the first where pred? is #t", lnfCond);
	lAddSpecialForm(c,"when",    "[cond ...body]",  "Evaluates BODY if CONDITION is #t", lnfWhen);
	lAddSpecialForm(c,"unless",  "[cond ...body]",  "Evaluates BODY if CONDITION is #f", lnfUnless);
	lAddSpecialForm(c,"and",     "[...args]",       "#t if all ARGS evaluate to true",   lnfAnd);
	lAddSpecialForm(c,"or" ,     "[...args]",       "#t if one member of ARGS evaluates to true", lnfOr);
	lAddSpecialForm(c,"while",   "[cond ...body]",  "Evaluate ...BODY for as long as COND is true, return the value of the last iteration of ...BODY or #nil when COND was false from the start", lnfWhile);
	lAddSpecialForm(c,"try",     "[catch ...body]", "Try evaluating ...BODY, and if an exception is thrown handle it using CATCH", lnfTry);

	lAddNativeFunc(c,"throw",   "[v]",             "Throw V to the closest exception handler", lnfThrow);
}
