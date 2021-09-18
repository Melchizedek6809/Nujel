/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "closure.h"
#include "../casting.h"
#include "../datatypes/closure.h"
#include "../datatypes/list.h"
#include "../datatypes/native-function.h"
#include "../datatypes/symbol.h"
#include "../datatypes/val.h"

static lVal *lnfDefine(uint c, lClosure *ec, lVal *v, lVal *(*func)(uint ,lSymbol *)){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = lCar(v);
	lVal *nv = NULL;
	if(lCdr(v) != NULL){
		nv = lEval(ec,lCadr(v));
	}
	if(sym->type != ltSymbol){sym = lEval(&lClo(c),sym);}
	if(sym->type != ltSymbol){return NULL;}
	lSymbol *lsym = lvSym(sym->vCdr);
	if((lsym != NULL) && (lsym->c[0] == ':')){return NULL;}
	lVal *t = func(c,lsym);
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	if((lCar(t) != NULL) && (lCar(t)->flags & lfConst)){
		return lCar(t);
	}else{
		t->vList.car = nv;
		return lCar(t);
	}
}

static lVal *lUndefineClosureSym(uint c, lVal *s){
	if(c == 0){return lValBool(false);}

	lVal *lastPair = lCloData(c);
	forEach(v,lCloData(c)){
		lVal *n = lCar(v);
		if((n == NULL) || (n->type != ltPair))  {break;}
		const lVal *sym = lCar(n);
		if(lSymCmp(s,sym) == 0){
			lastPair->vList.cdr = lCdr(v);
			return lValBool(true);
		}
		lastPair = v;
	}
	return lUndefineClosureSym(lCloParent(c),s);
}
static lVal *lnfUndef(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = lCar(v);
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	return lUndefineClosureSym(c - lClosureList,sym);
}
static lVal *lnfDef(lClosure *c, lVal *v){
	return lnfDefine(c - lClosureList,c,v,lDefineClosureSym);
}
static lVal *lnfSet(lClosure *c, lVal *v){
	return lnfDefine(c - lClosureList,c,v,lGetClosureSym);
}

static lVal *lSymTable(lClosure *c, lVal *v, int off, int len){
	(void)v;
	if((c == NULL) || (len == 0)){return v;}
	forEach(n,c->data){
		lVal *entry = lCadar(n);
		if(entry == NULL){continue;}
		if((entry->type != ltNativeFunc) && (entry->type != ltLambda)){continue;}

		if(off > 0){--off; continue;}
		v = lCons(lCaar(n),v);
		if(--len <= 0){return v;}
	}
	if(c->parent == 0){return v;}
	return lSymTable(&lClo(c->parent),v,off,len);
}

static lVal *lnfSymTable(lClosure *c, lVal *v){
	lVal *loff = lnfInt(c,lEval(c,lCar(v)));
	lVal *llen = lnfInt(c,lEval(c,lCadr(v)));
	int off = loff->vInt;
	int len = llen->vInt;
	if(len <= 0){len = 1<<16;}
	return lSymTable(c,NULL,off,len);
}

static int lSymCount(lClosure *c, int ret){
	if(c == NULL){return ret;}
	forEach(n,c->data){
		lVal *entry = lCadar(n);
		if(entry == NULL){continue;}
		if((entry->type != ltNativeFunc) && (entry->type != ltLambda)){continue;}
		++ret;
	}
	if(c->parent == 0){return ret;}
	return lSymCount(&lClo(c->parent),ret);
}

static lVal *lnfSymCount(lClosure *c, lVal *v){
	(void)v;
	return lValInt(lSymCount(c,0));
}

static lVal *lnfCl(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(v == NULL){return c->data != NULL ? c->data : lCons(NULL,NULL);}
	lVal *t = lnfInt(c,lEval(c,lCar(v)));
	if((t != NULL) && (t->type == ltInt) && (t->vInt > 0)){
		return lnfCl(&lClo(c->parent),lCons(lValInt(t->vInt - 1),NULL));
	}
	return c->data != NULL ? c->data : lCons(NULL,NULL);
}

static lVal *lnfClText(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,lCar(v));
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return lCloText(t->vCdr);
	}else if(t->type == ltNativeFunc){
		return lCons(lCdr(lNFN(t->vCdr).doc),NULL);
	}
	return NULL;
}

static lVal *lnfClData(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *t = lEval(c,lCar(v));
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return lCloData(t->vCdr);
	}else if(t->type == ltNativeFunc){
		return lCar(lNFN(t->vCdr).doc);
	}
	return NULL;
}

static lVal *lnfClLambda(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(v == NULL){return c->data != NULL ? c->data : lCons(NULL,NULL);}
	lVal *t = lnfInt(c,lEval(c,lCar(v)));
	if((t != NULL) && (t->type == ltInt) && (t->vInt > 0)){
		return lnfClLambda(&lClo(c->parent),lCons(lValInt(t->vInt - 1),NULL));
	}
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vCdr = c - lClosureList;
	c->refCount++;
	return ret;
}

static lVal *lnfLet(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	const uint nci = lClosureNew(c - lClosureList);
	if(nci == 0){return NULL;}
	lClosure *nc = &lClosureList[nci & CLO_MASK];
	forEach(n,lCar(v)){
		lnfDefine(nci,c,lCar(n),lDefineClosureSym);
	}
	lVal *ret = NULL;
	forEach(n,lCdr(v)){
		ret = lEval(nc,lCar(n));
	}
	c->refCount--;
	return ret == NULL ? NULL : ret;
}

void lOperationsClosure(lClosure *c){
	lAddNativeFunc(c,"resolve",        "[sym]",          "Resolve SYM until it is no longer a symbol", lResolve);
	lAddNativeFunc(c,"cl",             "[i]",            "Return closure",                             lnfCl);
	lAddNativeFunc(c,"cl-lambda",      "[i]",            "Return closure as a lambda",                 lnfClLambda);
	lAddNativeFunc(c,"cl-text",        "[f]",            "Return closures text segment",               lnfClText);
	lAddNativeFunc(c,"cl-data",        "[f]",            "Return closures data segment",               lnfClData);
	lAddNativeFunc(c,"symbol-table",   "[off len]",      "Return a list of len symbols defined, accessible from the current closure from offset off",lnfSymTable);
	lAddNativeFunc(c,"symbol-count",   "[]",             "Return a count of the symbols accessible from the current closure",lnfSymCount);

	lAddNativeFunc(c,"define def","[sym val]",     "Define a new symbol SYM and link it to value VAL",                 lnfDef);
	lAddNativeFunc(c,"undefine!", "[sym]",         "Remove symbol SYM from the first symbol-table it is found in",     lnfUndef);
	lAddNativeFunc(c,"set!",      "[s v]",         "Bind a new value v to already defined symbol s",                   lnfSet);
	lAddNativeFunc(c,"let",       "[args ...body]","Create a new closure with args bound in which to evaluate ...body",lnfLet);
}
