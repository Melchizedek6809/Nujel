/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "exception.h"
#include "api.h"

#include <stdlib.h>
#include <stdio.h>
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
	while(1);
}

/* Cause an exception, passing a list of SYMBOL and ERROR to the exception handler */
NORETURN void lExceptionThrow(const char *symbol, const char *error){
	lVal *l = lList(2, RVP(lValKeyword(symbol)), RVP(lValString(error)));
	lExceptionThrowRaw(l);
}

/* Cause an exception, passing a list of SYMBOL, ERROR and V to the exception handler */
NORETURN void lExceptionThrowVal(const char *symbol, const char *error, lVal *v){
	lVal *l = lList(3, RVP(lValKeyword(symbol)), RVP(lValString(error)),RVP(v));
	lExceptionThrowRaw(l);
}

/* Cause an exception, passing a list of SYMBOL, ERROR and V to the exception handler */
NORETURN void lExceptionThrowValClo(const char *symbol, const char *error, lVal *v, lClosure *c){
	lVal *l = lList(4, RVP(lValKeyword(symbol)), RVP(lValString(error)),RVP(v),RVP(lValLambda(c)));
	lExceptionThrowRaw(l);
}

static void exceptionCatchExit(lVal *exc){
	epf("Root Exception:\n%V\n",exc);
	exit(200);
}

/* Execute BODY(A,B) with a fallback exception handler set that writes everything to stdout before exiting. */
void *lExceptionTryExit(void *(*body)(void *,void *), void *a, void *b){
	return lExceptionTryCatch(body, a, b, exceptionCatchExit);
}

void *lExceptionTryCatch(void *(*body)(void *,void *), void *a, void *b, void (*handler)(lVal *exceptionValue)){
	jmp_buf oldExceptionTarget;
	memcpy(oldExceptionTarget,exceptionTarget,sizeof(jmp_buf));

	exceptionTargetDepth++;
	int ret = setjmp(exceptionTarget);
	if(ret){
		handler(exceptionValue);
		exceptionTargetDepth--;
		return NULL;
	}else{
		void *doRet = body(a, b);
		memcpy(exceptionTarget,oldExceptionTarget,sizeof(jmp_buf));
		exceptionTargetDepth--;
		return doRet;
	}
}
