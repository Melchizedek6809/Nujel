/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "collection/list.h"
#include "collection/string.h"
#include "display.h"
#include "exception.h"
#include "misc/pf.h"
#include "type/symbol.h"

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

jmp_buf exceptionTarget;
lVal *exceptionValue;
int exceptionTargetDepth = 0;

bool breakQueued = false;

/* Cause an exception, passing V directly to the closest exception handler */
NORETURN void lExceptionThrowRaw(lVal *v){
	if(exceptionTargetDepth <= 0){
		lWriteVal(v);
		exit(201);
	}
	exceptionValue = v;
	longjmp(exceptionTarget, 1);
	while(1){}
}

/* Cause an exception, passing a list of SYMBOL, ERROR and V to the exception handler */
NORETURN void lExceptionThrowValClo(const char *symbol, const char *error, lVal *v, lClosure *c){
	lVal *l = lList(4, RVP(lValKeyword(symbol)), RVP(lValString(error)),RVP(v),RVP(lValLambda(c)));
	lExceptionThrowRaw(l);
}
