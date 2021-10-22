/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "closure.h"
#include "special.h"
#include "../type-system.h"
#include "../allocation/roots.h"
#include "../collection/closure.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"

static lVal *lnfDefine(lClosure *c, lClosure *ec, lVal *v, lVal *(*func)(lClosure *,const lSymbol *)){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = lCar(v);
	const lSymbol *lsym = sym->vSymbol;
	if((lsym != NULL) && (lsym->c[0] == ':')){return NULL;}
	lVal *t = lRootsValPush(func(c,lsym));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	if(lCdr(v) == NULL){
		t->vList.car = NULL;
	}else{
		t->vList.car = lEval(ec,lCadr(v));
	}
	return lCar(lRootsValPop());
}

static lVal *lUndefineClosureSym(lClosure *c, lVal *s){
	if(c == NULL){return NULL;}
	lVal *lastPair = c->data;
	forEach(v,c->data){
		lVal *n = lCar(v);
		if((n == NULL) || (n->type != ltPair))  {break;}
		const lVal *sym = lCar(n);
		if(lSymCmp(s,sym) == 0){
			lastPair->vList.cdr = lCdr(v);
			return lValBool(true);
		}
		lastPair = v;
	}
	return lUndefineClosureSym(c->parent,s);
}
static lVal *lnfUndef(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *sym = lCar(v);
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	return lUndefineClosureSym(c,sym);
}
static lVal *lnfDef(lClosure *c, lVal *v){
	return lnfDefine(c,c,v,lDefineClosureSym);
}
static lVal *lnfSet(lClosure *c, lVal *v){
	return lnfDefine(c,c,v,lGetClosureSym);
}

static lVal *lSymTable(lClosure *c, lVal *v, int off, int len){
	(void)v;
	if((c == NULL) || (len == 0)){return v;}
	forEach(n,c->data){
		lVal *entry = lCadar(n);
		if(entry == NULL){continue;}
		if((entry->type != ltSpecialForm) && (entry->type != ltNativeFunc) && (entry->type != ltLambda)){continue;}

		if(off > 0){--off; continue;}
		v = lCons(lCaar(n),v);
		if(--len <= 0){return v;}
	}
	if(c->parent == 0){return v;}
	return lSymTable(c->parent,v,off,len);
}

static lVal *lnfSymTable(lClosure *c, lVal *v){
	int off = castToInt(lCar(v),0);
	int len = castToInt(lCadr(v),-1);
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
	return lSymCount(c->parent,ret);
}

static lVal *lnfSymCount(lClosure *c, lVal *v){
	(void)v;
	return lValInt(lSymCount(c,0));
}

static lVal *lCl(lClosure *c, int stepsLeft){
	if(c == NULL){return NULL;}
	if(stepsLeft > 0){
		return lCl(c->parent,stepsLeft-1);
	}
	return c->data != NULL ? c->data : lCons(NULL,NULL);
}

static lVal *lnfCl(lClosure *c, lVal *v){
	return lCl(c,castToInt(lCar(v),0));
}

static lVal *lnfClText(lClosure *c, lVal *v){
	(void)c;
	lVal *t = lCar(v);
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return t->vClosure->text;
	}else if(t->type == ltNativeFunc){
		return lCons(lCdr(t->vNFunc->doc),NULL);
	}
	return NULL;
}

static lVal *lnfClDoc(lClosure *c, lVal *v){
	(void)c;
	lVal *t = lCar(v);
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return t->vClosure->doc;
	}else if((t->type == ltNativeFunc) || (t->type == ltSpecialForm)){
		return t->vNFunc->doc;
	}
	return NULL;
}

static lVal *lnfClData(lClosure *c, lVal *v){
	(void)c;
	lVal *t = lCar(v);
	if(t == NULL){return NULL;}
	if(t->type == ltLambda){
		return lCar(t->vClosure->data);
	}else if(t->type == ltNativeFunc){
		return lCar(t->vNFunc->doc);
	}
	return NULL;
}

static lVal *lClLambda(lClosure *c, int stepsLeft){
	if(c == NULL){return NULL;}
	if(stepsLeft > 0){
		return lClLambda(c->parent,stepsLeft-1);
	}
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltLambda;
	ret->vClosure = c;
	return ret;
}

static lVal *lnfClLambda(lClosure *c, lVal *v){
	return lClLambda(c,castToInt(lCar(v),0));
}

static lVal *lnfLet(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lClosure *nc = lClosureNew(c);
	if(nc == NULL){return NULL;}
	forEach(n,lCar(v)){
		lnfDefine(nc,c,lCar(n),lDefineClosureSym);
	}
	lVal *ret = NULL;
	forEach(n,lCdr(v)){
		ret = lEval(nc,lCar(n));
	}
	return ret == NULL ? NULL : ret;
}

static lVal *lnfLetRaw(lClosure *c, lVal *v){
	return lnfDo(lClosureNew(c),v);
}

void lOperationsClosure(lClosure *c){
	lAddNativeFunc(c,"resolve",        "[sym]",         "Resolve SYM until it is no longer a symbol", lResolve);
	lAddNativeFunc(c,"cl",             "[i]",           "Return closure",                             lnfCl);
	lAddNativeFunc(c,"cl-lambda",      "[i]",           "Return closure as a lambda",                 lnfClLambda);
	lAddNativeFunc(c,"cl-text disasm", "[f]",           "Return closures text segment",               lnfClText);
	lAddNativeFunc(c,"cl-doc",         "[f]",           "Return documentation pair for F",            lnfClDoc);
	lAddNativeFunc(c,"cl-data",        "[f]",           "Return closures data segment",               lnfClData);
	lAddNativeFunc(c,"symbol-table",   "[off len]",     "Return a list of len symbols defined, accessible from the current closure from offset off",lnfSymTable);
	lAddNativeFunc(c,"symbol-count",   "[]",            "Return a count of the symbols accessible from the current closure",lnfSymCount);

	lAddSpecialForm(c,"define def",    "[sym val]",     "Define a new symbol SYM and link it to value VAL",                 lnfDef);
	lAddSpecialForm(c,"undefine!",     "[sym]",         "Remove symbol SYM from the first symbol-table it is found in",     lnfUndef);
	lAddSpecialForm(c,"set!",          "[s v]",         "Bind a new value v to already defined symbol s",                   lnfSet);
	lAddSpecialForm(c,"let",           "[args ...body]","Create a new closure with args bound in which to evaluate ...body",lnfLet);
	lAddSpecialForm(c,"let*",          "[...body]",     "Run body wihtin a new closure",lnfLetRaw);
}
