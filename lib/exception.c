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

__attribute__((noreturn)) void lExceptionThrow(const char *symbol, const char *error){
	lVal *l = lRootsValPush(lCons(NULL,NULL));
	lVal *c = l;

	c->vList.car = lValSym(symbol);
	c->vList.cdr = lCons(NULL,NULL);
	c = c->vList.cdr;
	c->vList.car = lValString(error);

	lExceptionThrowRaw(l);
}

__attribute__((noreturn)) void lExceptionThrowVal(const char *symbol, const char *error, lVal *v){
	lVal *l = lRootsValPush(lCons(NULL,NULL));
	lVal *c = l;

	c->vList.car = lValSym(symbol);
	c->vList.cdr = lCons(NULL,NULL);
	c = c->vList.cdr;
	c->vList.car = lValString(error);
	c->vList.cdr = lCons(NULL,NULL);
	c = c->vList.cdr;
	c->vList.car = v;

	lExceptionThrowRaw(l);
}

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
