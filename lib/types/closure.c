/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "closure.h"
#include "list.h"
#include "symbol.h"
#include "val.h"
#include "../nujel.h"

#ifndef COSMOPOLITAN_H_
	#include <ctype.h>
	#include <string.h>
#endif

lClosure  lClosureList[CLO_MAX];
uint      lClosureActive = 0;
uint      lClosureMax    = 1;
lClosure *lClosureFFree  = NULL;

void lInitClosure(){
	lClosureActive  = 0;
	lClosureMax     = 1;
}

lClosure *lClosureAlloc(){
	lClosure *ret;
	if(lClosureFFree == NULL){
		if(lClosureMax >= CLO_MAX-1){
			lPrintError("lClosure OOM ");
			return 0;
		}
		ret = &lClosureList[lClosureMax++];
	}else{
		ret = lClosureFFree;
		lClosureFFree = ret->nextFree;
	}
	lClosureActive++;
	*ret = (lClosure){0};
	ret->flags = lfUsed;
	return ret;
}

void lClosureFree(lClosure *clo){
	if((clo == NULL) || !(clo->flags & lfUsed)){return;}
	lClosureActive--;
	clo->nextFree = lClosureFFree;
	clo->flags    = 0;
	lClosureFFree = clo;
}

lClosure *lClosureNew(lClosure *parent){
	lClosure *c = lClosureAlloc();
	if(c == NULL){return NULL;}
	c->parent = parent;
	if(parent != NULL){
		parent->refCount++;
	}
	return c;
}

lVal *lSearchClosureSym(lClosure *c, lVal *ret, const char *str, uint len){
	if(c == 0){return ret;}

	forEach(n, c->data){
		lVal *e = lCaar(n);
		if((e == NULL) || (e->type != ltSymbol)){continue;}
		if(strncmp(e->vSymbol->c,str,len)){continue;}
		ret = lCons(e,ret);
	}
	return lSearchClosureSym(c->parent,ret,str,len);
}

lVal *lResolve(lClosure *c, lVal *v){
	v = lCar(v);
	for(int i=0;i<16;i++){
		if((v == NULL) || (v->type != ltSymbol)){break;}
		v = lResolveSym(c,v);
	}
	return v;
}

lVal *lResolveSym(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltSymbol)){return NULL;}
	lSymbol *sym = v->vSymbol;
	if(lSymKeyword(sym)){return v;}
	lVal *ret = lGetClosureSym(c,sym);
	return ret == NULL ? NULL : lCar(ret);
}

void lDefineVal(lClosure *c, const char *str, lVal *val){
	lVal *var = lDefineClosureSym(c,lSymS(str));
	if(var == NULL){return;}
	var->vList.car = val;
}

lVal *lDefineAliased(lClosure *c, lVal *lNF, const char *sym){
	const char *cur = sym;

	// Run at most 64 times, just a precaution
	for(int i=0;i<64;i++){
		uint len;
		for(len=0;len < sizeof(lSymbol);len++){ // Find the end of the current token, either space or 0
			if(cur[len] == 0)    {break;}
				if(isspace((u8)cur[len])){break;}
		}
		lVal *var = lDefineClosureSym(c,lSymSL(cur,len));
		if(var == NULL){
			lPrintError("Error adding NFunc %s\n",sym);
			return NULL;
		}
		var->vList.car = lNF;
		for(;len<32;len++){ // Advance to the next non whitespace character
			if(cur[len] == 0)     {return lNF;} // Or return if we reached the final 0 byte
			if(!isspace((u8)cur[len])){break;}
		}
		cur += len;
	}
	lPrintError("Quite the amount of aliases we have there (%s)\n",sym);
	return NULL;
}

static lVal *lGetSym(lClosure *c, lSymbol *s){
	if((c == 0) || (s == NULL)){return NULL;}
	forEach(v, c->data){
		lVal *cursym = lCaar(v);
		if((cursym == NULL) || (s != cursym->vSymbol)){continue;}
		return lCdar(v);
	}
	return NULL;
}

lVal *lGetClosureSym(lClosure *c, lSymbol *s){
	if(c == 0){return NULL;}
	lVal *t = lGetSym(c,s);
	return t != NULL ? t : lGetClosureSym(c->parent,s);
}

lVal *lDefineClosureSym(lClosure *c, lSymbol *s){
	if(c == 0){return NULL;}
	lVal *get = lGetSym(c,s);
	if(get != NULL){return get;}
	lVal *t = lCons(lValSymS(s),lCons(NULL,NULL));
	if(t == NULL){return NULL;}
	if(c->data == NULL){
		c->data = lCons(t,NULL);
	}else{
		lVal *cdr = NULL;
		for(cdr = c->data;(cdr != NULL) && (lCdr(cdr) != NULL);cdr = lCdr(cdr)){}
		if(cdr == NULL){return NULL;}
		cdr->vList.cdr = lCons(t,NULL);
	}
	return t->vList.cdr;
}

int lClosureID(const lClosure *n){
	return n - lClosureList;
}
