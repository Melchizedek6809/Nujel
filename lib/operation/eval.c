/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "eval.h"

#include "../api.h"

/* [eval* expr] - Evaluate the already compiled EXPR */
static lVal *lnfEvalRaw(lClosure *c, lVal *v){
	return lEval(c,lCar(v));
}

/* Handler for [apply fn list] */
static lVal *lnfApply(lClosure *c, lVal *v){
	lVal *fun = lCar(v);
	if(fun == NULL){return NULL;}
	if(fun->type == ltSymbol){
		fun = lResolveSym(c,fun);
	}
	return lApply(c,lCadr(v),fun);
}

/* Handler for [apply fn list] */
static lVal *lnfMacroApply(lClosure *c, lVal *v){
	lVal *fun = lCar(v);
	if(fun == NULL){return NULL;}
	if(fun->type == ltSymbol){ fun = lResolveSym(c,fun); }
	if(fun->type != ltMacro){
		lExceptionThrow(":not-a-macro","Trying to use macro-apply on anything but a macro is an error, please fix it");
	}

	return lLambda(c,lCadr(v),fun);
}

void lOperationsEval(lClosure *c){
	lAddNativeFunc(c,"apply", "[func list]", "Evaluate FUNC with LIST as arguments", lnfApply);
	lAddNativeFunc(c,"macro-apply", "[macro list]", "Evaluate MACRO with LIST as arguments", lnfMacroApply);
	lAddNativeFunc(c,"eval*", "[expr]",      "Evaluate the already compiled EXPR",   lnfEvalRaw);
}
