/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"

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
	lVal *res = NULL;
	for(lVal *t=v;t;t = t->vList.cdr){
		res = lEval(c,lCar(t));
		if(!castToBool(res)){break;}
	}
	return res;
}

static lVal *lnfOr(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lEval(c,lCar(v));
	return castToBool(t) ? t : lnfOr(c,lCdr(v));
}

static lVal *lnfIf(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	const bool pred = castToBool(lEval(c,lCar(v)));
	return lEval(c,pred ? lCadr(v) : lCaddr(v));
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	(void)c;
	if(v->type == ltPair){
		return lCar(v);
	}else {
		lExceptionThrowValClo(":invalid-quote","Quote needs a second argument to return, maybe you were trying to use a dotted pair instead of a list?", v, c);
		return NULL;
	}
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

static lVal *lnfWhile(lClosure *c, lVal *v){
	lVal *cond = lCar(v);
	lVal *body = lCdr(v);
	lVal *ret  = NULL;
	const int SP = lRootsGet();
	while(castToBool(lEval(c,cond))){
		lRootsRet(SP);
		ret = RVP(lnfDo(c,body));
	}
	lRootsRet(SP);
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

static lVal *lnfThrow(lClosure *c, lVal *v){
	(void)c;
	lExceptionThrowRaw(lCar(v));
	return NULL;
}

void lOperationsSpecial(lClosure *c){
	lnfvDo    = lAddSpecialForm(c,"do",    "[...body]",        "Evaluate ...body in order and returns the last result", lnfDo);
	lnfvQuote = lAddSpecialForm(c,"quote", "[v]",              "Return v as is without evaluating", lnfQuote);
	            lAddSpecialForm(c,"if",    "[cond then else]", "Evalute then if pred? is #t, otherwise evaluates ...else", lnfIf);
	            lAddSpecialForm(c,"and",   "[...args]",        "#t if all ARGS evaluate to true",   lnfAnd);
	            lAddSpecialForm(c,"or" ,   "[...args]",        "#t if one member of ARGS evaluates to true", lnfOr);
	            lAddSpecialForm(c,"while", "[cond ...body]",   "Evaluate ...BODY for as long as COND is true, return the value of the last iteration of ...BODY or #nil when COND was false from the start", lnfWhile);
	            lAddSpecialForm(c,"try",   "[catch ...body]",  "Try evaluating ...BODY, and if an exception is thrown handle it using CATCH", lnfTry);

	lAddNativeFunc(c,"throw",   "[v]",             "Throw V to the closest exception handler", lnfThrow);
}
