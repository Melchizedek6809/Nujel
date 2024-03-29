/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

char *errorSym = "vm-error";
#define lVMErrorPlaceholder(sym) \
	static lVal sym (){\
		return lValException(lSymVMError, "Gotta use bytecode " #sym, NIL); \
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
lVMErrorPlaceholder(lnfList)
lVMErrorPlaceholder(lnfThrow)
lVMErrorPlaceholder(lnfApply)

void lOperationsSpecial(){
	lAddNativeFunc("do",              "body",                    "Evaluate body in order and returns the last result", lnfDo, 0);
	lAddNativeFunc("let*",            "body",                    "Run BODY wihtin a new closure",  lnfLetRaw, 0);
	lAddNativeFunc("if",              "(cond then else)",        "Evalute then if pred? is #t, otherwise evaluates ...else", lnfIf, 0);
	lAddNativeFunc("and",             "args",                    "#t if all ARGS evaluate to true",   lnfAnd, 0);
	lAddNativeFunc("or" ,             "args",                    "#t if one member of ARGS evaluates to true", lnfOr, 0);
	lAddNativeFunc("while",           "(cond . body)",           "Evaluate BODY while COND is #t", lnfWhile, 0);
	lAddNativeFunc("try",             "(catch . body)",          "Try evaluating ...BODY, and if an exception is thrown handle it using CATCH", lnfTry, 0);
	lAddNativeFunc("return",          "(v)",                     "Do an early return with V", lnfReturn, 0);
	lAddNativeFunc("def",             "(sym val)",               "Define a new symbol SYM and link it to value VAL", lnfDef, 0);
	lAddNativeFunc("set!",            "(s v)",                   "Bind a new value v to already defined symbol s",   lnfSet, 0);
	lAddNativeFunc("macro*",          "(name args source body)", "Create a new, bytecoded, macro", lnfMacro, 0);
	lAddNativeFunc("fn*",             "(name args source body)", "Create a new, bytecoded, lambda", lnfLambda, 0);
	lAddNativeFunc("environment*",    "",                      "Create a new object",       lnfEnvironment, 0);
	lAddNativeFunc("bytecode-eval*",  "(bc-arr env)",            "Evaluate BC-ARR in call closure ofENV", lnfBytecodeEval, 0);
	lAddNativeFunc("mutable-eval*",   "(bc-arr env)",            "Evaluate BC-ARR directly in ENV", lnfMutableEval, 0);
	lAddNativeFunc("list",            "arguments",               "Return ARGUMENTS as a list", lnfList, 0);
	lAddNativeFunc("ref",             "(collection key)",        "Look up key in collection", lnfRef, NFUNC_PURE);
	lAddNativeFunc("throw",           "(v)",                     "Throw V to the closest exception handler", lnfThrow, 0);
	lAddNativeFunc("apply",           "(func list)",             "Evaluate FUNC with LIST as arguments",  lnfApply, NFUNC_PURE);
}
