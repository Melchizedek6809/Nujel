/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../type/closure.h"

char *errorSym = "vm-error";
char *errorMsg = "Gotta use bytecode";
#define lVMErrorPlaceholder(sym) \
	static lVal * sym (lClosure *c, lVal *v){			\
		lExceptionThrowValClo(errorSym, errorMsg, v, c);\
		return NULL;					\
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
lVMErrorPlaceholder(lnfObjectAst)

void lOperationsSpecial(lClosure *c){
	lAddNativeFunc(c,"do",               "body",                    "Evaluate body in order and returns the last result", lnfDo);
	lAddNativeFunc(c,"let*",             "body",                    "Run BODY wihtin a new closure",  lnfLetRaw);
	lAddNativeFunc(c,"if",               "[cond then else]",        "Evalute then if pred? is #t, otherwise evaluates ...else", lnfIf);
	lAddNativeFunc(c,"and",              "args",                    "#t if all ARGS evaluate to true",   lnfAnd);
	lAddNativeFunc(c,"or" ,              "args",                    "#t if one member of ARGS evaluates to true", lnfOr);
	lAddNativeFunc(c,"while",            "[cond . body]",           "Evaluate BODY while COND is #t", lnfWhile);
	lAddNativeFunc(c,"try",              "[catch . body]",          "Try evaluating ...BODY, and if an exception is thrown handle it using CATCH", lnfTry);
	lAddNativeFunc(c,"return",           "[v]",                     "Do an early return with V", lnfReturn);
	lAddNativeFunc(c,"def",              "[sym val]",               "Define a new symbol SYM and link it to value VAL", lnfDef);
	lAddNativeFunc(c,"set!",             "[s v]",                   "Bind a new value v to already defined symbol s",   lnfSet);
	lAddNativeFunc(c,"macro* macron*",          "[name args source body]", "Create a new, bytecoded, macro", lnfMacro);
	lAddNativeFunc(c,"fn* fnn*",             "[name args source body]", "Create a new, bytecoded, lambda", lnfLambda);
	lAddNativeFunc(c,"ω* environment*", "[]",                      "Create a new object",       lnfObjectAst);
}
