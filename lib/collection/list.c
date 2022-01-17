/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "list.h"
#include "../allocation/val.h"
#include "../type/val.h"

#include <stdarg.h>

/* Create and return a cell out of CAR and CDR */
lVal *lCons(lVal *car, lVal *cdr){
	lVal *v = lValAlloc();
	v->type = ltPair;
	v->vList.car = car;
	v->vList.cdr = cdr;
	return v;
}

/* Return the length of the list V */
int lListLength(lVal *v){
	int i = 0;
	for(lVal *n = v;(n != NULL) && (lCar(n) != NULL); n = lCdr(n)){i++;}
	return i;
}

/* A convenient way to generate a list */
lVal *lList(int length, ...){
	lVal *ret = NULL, *l;
	va_list varArgs;
	va_start(varArgs,length);
	for(;length;length--){
		lVal *t = va_arg(varArgs, lVal *);
		if(ret == NULL){
			ret = l = RVP(lCons(NULL,NULL));
		}else{
			l = l->vList.cdr = lCons(NULL,NULL);
		}
		l->vList.car = t;
	}
	va_end(varArgs);
	return ret;
}
