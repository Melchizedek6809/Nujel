/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "symbol.h"
#include "val.h"
#include "../display.h"
#include "../nujel.h"
#include "../allocation/symbol.h"
#include "../allocation/val.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Return a newly allocated nujel symbol of value S */
lVal *lValSymS(const lSymbol *s){
	if(s == NULL){return NULL;}
	lVal *ret = lValAlloc();
	ret->type = ltSymbol;
	ret->vSymbol = s;
	return ret;
}

/* Return a nujel value for the symbol within S */
lVal *lValSym(const char *s){
	return lValSymS(lSymS(s));
}

/* Return true if S is a symbol for a variadic argument */
bool lSymVariadic(const lSymbol *s){
	const char *p = s->c;
	if((p[0] == '.') && (p[1] == '.') && (p[2] == '.')){
		return true;
	}
	return false;
}

/* Return true if S is a keyword symbol */
bool lSymKeyword(const lSymbol *s){
	return s->c[0] == ':';
}

/* Search the global symbol table for STR */
lVal *lSymbolSearch(const char *str, uint len){
	lVal *ret,*l;
	ret = l = NULL;
	if(str == NULL){return NULL;}
	for(uint i=0;i<lSymbolMax;i++){
		if(strncmp(lSymbolList[i].c,str,len)){continue;}
		if(l == NULL){
			ret = l = lRootsValPush(lCons(NULL,NULL));
		}else{
			l = l->vList.cdr = lCons(NULL,NULL);
		}
		l->vList.car = lValSymS(&lSymbolList[i]);
	}
	return ret;
}
