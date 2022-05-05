/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../exception.h"
#include "../type/native-function.h"
#include "../type/val.h"

lVal *lnfvDo;
lVal *lnfvQuote;

static lVal *lnfAnd(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [and] anymore", v, c);
	return NULL;
}

static lVal *lnfOr(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [or] anymore", v, c);
	return NULL;
}

static lVal *lnfIf(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [if] anymore", v, c);
	return NULL;
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	if(v->type == ltPair){
		return lCar(v);
	}else {
		lExceptionThrowValClo("invalid-quote","Quote needs a second argument to return, maybe you were trying to use a dotted pair instead of a list?", v, c);
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
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [while] anymore", v, c);
	return NULL;
}

lVal *lnfTry(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [try] anymore", v, c);
	return NULL;
}

static lVal *lnfThrow(lClosure *c, lVal *v){
	(void)c;
	lExceptionThrowRaw(lCar(v));
	return NULL;
}

static lVal *lnfReturn(lClosure *c, lVal *v){
	lExceptionThrowValClo("vm-error", "Can't return via apply, only the bytecode VM can do that", v, c);
	return NULL;
}

void lOperationsSpecial(lClosure *c){
	lnfvDo    = lAddSpecialForm(c,"do",    "body",             "Evaluate body in order and returns the last result", lnfDo);
	lnfvQuote = lAddSpecialForm(c,"quote", "[v]",              "Return v as is without evaluating", lnfQuote);
	            lAddSpecialForm(c,"if",    "[cond then else]", "Evalute then if pred? is #t, otherwise evaluates ...else", lnfIf);
	            lAddSpecialForm(c,"and",   "args",             "#t if all ARGS evaluate to true",   lnfAnd);
	            lAddSpecialForm(c,"or" ,   "args",             "#t if one member of ARGS evaluates to true", lnfOr);
	            lAddSpecialForm(c,"while", "[cond . body]",    "Evaluate ...BODY for as long as COND is true, return the value of the last iteration of ...BODY or #nil when COND was false from the start", lnfWhile);
	            lAddSpecialForm(c,"try",   "[catch . body]",   "Try evaluating ...BODY, and if an exception is thrown handle it using CATCH", lnfTry);
	            lAddSpecialForm(c,"return","[v]",              "Do an early return with V", lnfReturn);

	lAddNativeFunc(c,"throw",   "[v]",             "Throw V to the closest exception handler", lnfThrow);
}
