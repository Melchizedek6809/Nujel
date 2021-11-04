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


void lExceptionThrowRaw(lVal *v){
	exceptionValue = v;
	longjmp(exceptionTarget, 1);
}

void lExceptionThrow(const char *symbol, const char *error){
	lVal *l = lRootsValPush(lCons(NULL,NULL));
	lVal *c = l;

	c->vList.car = lValSym(symbol);
	c->vList.cdr = lCons(NULL,NULL);
	c = c->vList.cdr;
	c->vList.car = lValString(error);

	lExceptionThrowRaw(l);
}

void lExceptionThrowVal(const char *symbol, const char *error, lVal *v){
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

void lExceptionInit(){
	int ret;
	exceptionValue = NULL;
	ret = setjmp(exceptionTarget);
	if(ret){
		lWriteVal(exceptionValue);
		fprintf(stderr,"Unhandled exception, exiting immediately!\n");
		exit(1);
	}
}
