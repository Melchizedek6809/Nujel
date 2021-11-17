/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
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

/* Cause an exception, passing V directly to the closest exception handler */
__attribute__((noreturn)) void lExceptionThrowRaw(lVal *v){
	if(exceptionTargetDepth <= 0){
		lPrintError("Exception without a handler!!! Exiting!\n");
		lWriteVal(v);
		exit(201);
	}
	exceptionValue = v;
	longjmp(exceptionTarget, 1);
	while(1);
}

/* Cause an exception, passing a list of SYMBOL and ERROR to the exception handler */
__attribute__((noreturn)) void lExceptionThrow(const char *symbol, const char *error){
	lVal *l = lList(2,RVP(lValSym(symbol)),RVP(lValString(error)));
	lExceptionThrowRaw(l);
}

/* Cause an exception, passing a list of SYMBOL, ERROR and V to the exception handler */
__attribute__((noreturn)) void lExceptionThrowVal(const char *symbol, const char *error, lVal *v){
	lVal *l = lList(3,RVP(lValSym(symbol)),RVP(lValString(error)),RVP(v));
	lExceptionThrowRaw(l);
}

/* Execute BODY(A,B) with a fallback exception handler set that writes everything to stdout before exiting. */
void *lExceptionTry(void *(*body)(void *,void *), void *a, void *b){
	jmp_buf oldExceptionTarget;
	memcpy(oldExceptionTarget,exceptionTarget,sizeof(jmp_buf));

	exceptionTargetDepth++;
	int ret = setjmp(exceptionTarget);
	if(ret){
		lWriteVal(exceptionValue);
		exceptionTargetDepth--;
		exit(200);
		return NULL;
	}else{
		void *doRet = body(a, b);
		memcpy(exceptionTarget,oldExceptionTarget,sizeof(jmp_buf));
		exceptionTargetDepth--;
		return doRet;
	}
}
