/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../collection/list.h"
#include "../type/closure.h"
#include "../type/native-function.h"
#include "../exception.h"

/* Handler for [apply fn list] */
static lVal *lnfApply(lClosure *c, lVal *v){
	lVal *fun = lCar(v);
	switch(fun ? fun->type : ltNoAlloc){
	default:
		lExceptionThrowValClo("type-error", "Can't apply to that", v, c);
		return NULL;
	case ltObject:
	case ltLambda:
	case ltNativeFunc:
	case ltSpecialForm:
		return lApply(c, lCadr(v), fun, fun);
	}
}

/* Handler for [apply fn list] */
static lVal *lnfMacroApply(lClosure *c, lVal *v){
	lVal *fun = lCar(v);
	if((fun == NULL) || (fun->type != ltMacro)){
		lExceptionThrowValClo("type-error", "Can't macro-apply to that", v, c);
		return NULL;
	}
	return lLambda(c, lCadr(v), fun);
}

void lOperationsEval(lClosure *c){
	lAddNativeFunc(c,"apply",       "[func list]",  "Evaluate FUNC with LIST as arguments",  lnfApply);
	lAddNativeFunc(c,"macro-apply", "[macro list]", "Evaluate MACRO with LIST as arguments", lnfMacroApply);
}
