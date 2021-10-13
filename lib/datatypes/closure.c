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

#include <ctype.h>
#include <string.h>

lClosure lClosureList[CLO_MAX];
uint     lClosureActive = 0;
uint     lClosureMax    = 1;
uint     lClosureFFree  = 0;

void lInitClosure(){
	lClosureActive  = 0;
	lClosureMax     = 1;
}

uint lClosureAlloc(){
	lClosure *ret;
	if(lClosureFFree == 0){
		if(lClosureMax >= CLO_MAX-1){
			lPrintError("lClosure OOM ");
			return 0;
		}
		ret = &lClosureList[lClosureMax++];
	}else{
		ret = &lClosureList[lClosureFFree & CLO_MASK];
		lClosureFFree = ret->nextFree;
	}
	lClosureActive++;
	*ret = (lClosure){0};
	ret->flags = lfUsed;
	return ret - lClosureList;
}

void lClosureFree(uint c){
	if((c == 0) || (c >= lClosureMax)){return;}
	lClosure *clo = &lClosureList[c];
	if(!(clo->flags & lfUsed)){return;}
	lClosureActive--;
	clo->nextFree   = lClosureFFree;
	clo->flags      = 0;
	lClosureFFree = c;
}

uint lClosureNew(uint parent){
	const uint i = lClosureAlloc();
	if(i == 0){return 0;}
	lClosure *c = &lClosureList[i];
	c->parent = parent;
	lClosure *p = &lClosureList[parent & CLO_MASK];
	p->refCount++;
	return i;
}

lVal *lSearchClosureSym(uint c, lVal *ret, const char *str, uint len){
	if(c == 0){return ret;}

	forEach(n,lCloData(c)){
		lVal *e = lCaar(n);
		if((e == NULL) || (e->type != ltSymbol)){continue;}
		lSymbol *sym = lvSym(e->vCdr);
		if(sym == NULL){continue;}
		if(strncmp(sym->c,str,len)){continue;}
		ret = lCons(e,ret);
	}
	return lSearchClosureSym(lCloParent(c),ret,str,len);
}

lVal *lResolve(lClosure *c, lVal *v){
	v = lCar(v);
	for(int i=0;i<16;i++){
		if((v == NULL) || (v->type != ltSymbol)){break;}
		v = lResolveSym(c - lClosureList,v);
	}
	return v;
}

lVal *lResolveSym(uint c, lVal *v){
	if((v == NULL) || (v->type != ltSymbol)){return NULL;}
	lSymbol *sym = lvSym(v->vCdr);
	if(lSymKeyword(sym)){return v;}
	lVal *ret = lGetClosureSym(c,sym);
	return ret == NULL ? NULL : lCar(ret);
}

void lDefineVal(lClosure *c, const char *str, lVal *val){
	lVal *var = lDefineClosureSym(lCloI(c),lSymS(str));
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
		lVal *var = lDefineClosureSym(lCloI(c),lSymSL(cur,len));
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

static lVal *lGetSym(uint c, lSymbol *s){
	if((c == 0) || (s == NULL)){return NULL;}
	uint sym = lvSymI(s);
	forEach(v,lCloData(c)){
		lVal *cursym = lCaar(v);
		if((cursym == NULL) || (sym != cursym->vCdr)){continue;}
		return lCdar(v);
	}
	return NULL;
}

lVal *lGetClosureSym(uint c, lSymbol *s){
	if(c == 0){return NULL;}
	lVal *t = lGetSym(c,s);
	return t != NULL ? t : lGetClosureSym(lCloParent(c),s);
}

lVal *lDefineClosureSym(uint c, lSymbol *s){
	if(c == 0){return NULL;}
	lVal *get = lGetSym(c,s);
	if(get != NULL){return get;}
	lVal *t = lCons(lValSymS(s),lCons(NULL,NULL));
	if(t == NULL){return NULL;}
	if(lCloData(c) == NULL){
		lCloData(c) = lCons(t,NULL);
	}else{
		lVal *cdr = NULL;
		for(cdr = lCloData(c);(cdr != NULL) && (lCdr(cdr) != NULL);cdr = lCdr(cdr)){}
		if(cdr == NULL){return NULL;}
		cdr->vList.cdr = lCons(t,NULL);
	}
	return t->vList.cdr;
}
