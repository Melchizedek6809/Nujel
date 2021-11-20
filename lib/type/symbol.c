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

lVal *lValSymS(const lSymbol *s){
	if(s == NULL){return NULL;}
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltSymbol;
	ret->vSymbol = s;
	return ret;
}

lVal *lValSym(const char *s){
	return lValSymS(lSymS(s));
}

 bool lSymVariadic(const lSymbol *s){
	const char *p = s->c;
	if((*p == '@') || (*p == '&')){p++;}
	if((*p == '@') || (*p == '&')){p++;}
	if((p[0] == '.') && (p[1] == '.') && (p[2] == '.')){
		return true;
	}
	return false;
}

bool lSymKeyword(const lSymbol *s){
	return s->c[0] == ':';
}

int lSymCmp(const lVal *a,const lVal *b){
	if((a == NULL) || (b == NULL)){return 2;}
	if((a->type != ltSymbol) || (b->type != ltSymbol) || (a->vSymbol == NULL)){return 2;}
	return a->vSymbol == b->vSymbol ? 0 : -1;
}

int lSymEq(const lSymbol *a,const lSymbol *b){
	return a == b ? 0 : -1;
}

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
