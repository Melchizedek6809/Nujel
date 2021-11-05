/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "exception.h"
#include "api.h"

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

jmp_buf exceptionTarget;
lVal *exceptionValue;


__attribute__((noreturn)) void lExceptionThrowRaw(lVal *v){
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
