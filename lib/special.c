/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

char *errorSym = "vm-error";
char *errorMsg = "Gotta use bytecode";
#define lVMErrorPlaceholder(sym) \
	static lVal sym (){\
		return lValException(lSymVMError, errorMsg, NIL); \
	}

lVMErrorPlaceholder(lnfAnd)
lVMErrorPlaceholder(lnfOr)
lVMErrorPlaceholder(lnfIf)
lVMErrorPlaceholder(lnfWhile)
lVMErrorPlaceholder(lnfTry)
lVMErrorPlaceholder(lnfDo)
lVMErrorPlaceholder(lnfReturn)
lVMErrorPlaceholder(lnfDef)
lVMErrorPlaceholder(lnfSet)
lVMErrorPlaceholder(lnfLetRaw)
lVMErrorPlaceholder(lnfLambda)
lVMErrorPlaceholder(lnfMacro)
lVMErrorPlaceholder(lnfEnvironment)
lVMErrorPlaceholder(lnfBytecodeEval)
lVMErrorPlaceholder(lnfMutableEval)
lVMErrorPlaceholder(lnfRef)
lVMErrorPlaceholder(lnfCadr)
lVMErrorPlaceholder(lnfList)
lVMErrorPlaceholder(lnfThrow)
lVMErrorPlaceholder(lnfApply)

void lOperationsSpecial(lClosure *c){
	lAddNativeFunc(c,"do",              "body",                    "Evaluate body in order and returns the last result", lnfDo, 0);
	lAddNativeFunc(c,"let*",            "body",                    "Run BODY wihtin a new closure",  lnfLetRaw, 0);
	lAddNativeFunc(c,"if",              "(cond then else)",        "Evalute then if pred? is #t, otherwise evaluates ...else", lnfIf, 0);
	lAddNativeFunc(c,"and",             "args",                    "#t if all ARGS evaluate to true",   lnfAnd, 0);
	lAddNativeFunc(c,"or" ,             "args",                    "#t if one member of ARGS evaluates to true", lnfOr, 0);
	lAddNativeFunc(c,"while",           "(cond . body)",           "Evaluate BODY while COND is #t", lnfWhile, 0);
	lAddNativeFunc(c,"try",             "(catch . body)",          "Try evaluating ...BODY, and if an exception is thrown handle it using CATCH", lnfTry, 0);
	lAddNativeFunc(c,"return",          "(v)",                     "Do an early return with V", lnfReturn, 0);
	lAddNativeFunc(c,"def",             "(sym val)",               "Define a new symbol SYM and link it to value VAL", lnfDef, 0);
	lAddNativeFunc(c,"set!",            "(s v)",                   "Bind a new value v to already defined symbol s",   lnfSet, 0);
	lAddNativeFunc(c,"macro*",          "(name args source body)", "Create a new, bytecoded, macro", lnfMacro, 0);
	lAddNativeFunc(c,"fn*",             "(name args source body)", "Create a new, bytecoded, lambda", lnfLambda, 0);
	lAddNativeFunc(c,"environment*",    "()",                      "Create a new object",       lnfEnvironment, 0);
	lAddNativeFunc(c,"bytecode-eval*",  "(bc-arr env)",            "Evaluate BC-ARR in call closure ofENV", lnfBytecodeEval, 0);
	lAddNativeFunc(c,"mutable-eval*",   "(bc-arr env)",            "Evaluate BC-ARR directly in ENV", lnfMutableEval, 0);
	lAddNativeFunc(c,"list",            "arguments",               "Return ARGUMENTS as a list", lnfList, 0);
	lAddNativeFunc(c,"ref",             "(collection key)",        "Look up key in collection", lnfRef, NFUNC_PURE);
	lAddNativeFunc(c,"cadr",            "(list)",                  "Look up the cadr of list", lnfCadr, NFUNC_PURE);
	lAddNativeFunc(c,"throw",           "(v)",                     "Throw V to the closest exception handler", lnfThrow, 0);
	lAddNativeFunc(c,"apply",           "(func list)",             "Evaluate FUNC with LIST as arguments",  lnfApply, NFUNC_PURE);
}
