/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "closure.h"
#include "list.h"
#include "tree.h"
#include "../nujel.h"
#include "../allocation/garbage-collection.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <ctype.h>
#include <string.h>

lClosure  lClosureList[CLO_MAX];
uint      lClosureActive = 0;
uint      lClosureMax    = 0;
lClosure *lClosureFFree  = NULL;

void lInitClosure(){
	lClosureActive  = 0;
	lClosureMax     = 0;
}

lClosure *lClosureAlloc(){
	lClosure *ret;
	if(lClosureFFree == NULL){
		if(lClosureMax >= CLO_MAX-1){
			lGarbageCollect();
			if(lClosureFFree == NULL){
				lPrintError("lClosure OOM ");
				return 0;
			}else{
				ret = lClosureFFree;
				lClosureFFree = ret->nextFree;
			}
		}else{
			ret = &lClosureList[lClosureMax++];
		}
	}else{
		ret = lClosureFFree;
		lClosureFFree = ret->nextFree;
	}
	lClosureActive++;
	*ret = (lClosure){0};
	return ret;
}

void lClosureFree(lClosure *clo){
	if(clo == NULL){return;}
	lClosureActive--;
	clo->nextFree = lClosureFFree;
	lClosureFFree = clo;
}

lClosure *lClosureNew(lClosure *parent){
	lClosure *c = lClosureAlloc();
	if(c == NULL){return NULL;}
	c->parent = parent;
	return c;
}

lVal *lSearchClosureSym(lClosure *c, lVal *ret, const char *str, uint len){
	(void)str;(void)len;
	if(c == NULL){return ret;}
	return NULL;/*
	forEach(n, c->data){
		lVal *e = lCaar(n);
		if((e == NULL) || (e->type != ltSymbol)){continue;}
		if(strncmp(e->vSymbol->c,str,len)){continue;}
		ret = lCons(e,ret);
	}
	return lSearchClosureSym(c->parent,ret,str,len);*/
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
	const lSymbol *sym = v->vSymbol;
	if(lSymKeyword(sym)){return v;}
	lVal *ret = lGetClosureSym(c,sym);
	return ret == NULL ? NULL : ret;
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
		lDefineClosureSym(c,lSymSL(cur,len),lNF);
		for(;len<32;len++){ // Advance to the next non whitespace character
			if(cur[len] == 0)     {return lNF;} // Or return if we reached the final 0 byte
			if(!isspace((u8)cur[len])){break;}
		}
		cur += len;
	}
	lPrintError("Quite the amount of aliases we have there (%s)\n",sym);
	return NULL;
}

static lVal *lGetSym(lClosure *c, const lSymbol *s){
	if((c == NULL) || (s == NULL)){return NULL;}
	return lTreeGet(c->data,s,NULL);
}

lVal *lGetClosureSym(lClosure *c,const lSymbol *s){
	if(c == NULL){return NULL;}
	lVal *t = lGetSym(c,s);
	return t != NULL ? t : lGetClosureSym(c->parent,s);
}

void lDefineClosureSym(lClosure *c,const lSymbol *s, lVal *v){
	c->data = lTreeInsert(c->data,s,v);
}

void lSetClosureSym(lClosure *c,const lSymbol *s, lVal *v){
	if(c == NULL){return;}
	bool found = false;
	lTreeSet(c->data,s,v,&found);
	if(!found){
		lSetClosureSym(c->parent,s,v);
	}
}

void lDefineVal(lClosure *c, const char *str, lVal *val){
	lDefineClosureSym(c,lSymS(str),val);
}

int lClosureID(const lClosure *n){
	return n - lClosureList;
}
