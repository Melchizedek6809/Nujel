/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "closure.h"
#include "special.h"
#include "../type-system.h"
#include "../allocation/roots.h"
#include "../allocation/val.h"
#include "../collection/closure.h"
#include "../collection/list.h"
#include "../collection/tree.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"


static lVal *lnfDef(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *sym = lCar(v);
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	const lSymbol *s = sym->vSymbol;

	lVal *ret = lEval(c,lCadr(v));
	lDefineClosureSym(c,s,ret);
	return ret;
}

static lVal *lnfSet(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *sym = lCar(v);
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	const lSymbol *s = sym->vSymbol;

	lVal *ret = lEval(c,lCadr(v));
	lSetClosureSym(c,s,ret);
	return ret;
}

static lVal *lSymTable(lClosure *c, lVal *v){
	if(c == NULL){return v;}
	lRootsValPush(v);
	v = lTreeAddKeysToList(c->data,v);
	return lSymTable(c->parent,v);
}

static lVal *lnfSymbolTable(lClosure *c, lVal *v){
	(void)v;
	return lSymTable(c,NULL);
}

static int lSymCount(lClosure *c, int ret){
	if(c == NULL){return ret;}
	return lSymCount(c->parent,lTreeSize(c->data) + ret);
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
	return c->data != NULL ? lValTree(c->data) : lCons(NULL,NULL);
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
		return lValTree(t->vClosure->data);
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
	int SP = lRootsGet();
	lClosure *nc = lRootsClosurePush(lClosureNew(c));
	forEach(n,lCar(v)){
		lVal *sym = lCaar(n);
		if((sym == NULL) || (sym->type != ltSymbol)){continue;}
		lDefineClosureSym(nc,sym->vSymbol,lEval(c,lCadar(n)));
	}
	lVal *ret = NULL;
	forEach(n,lCdr(v)){
		ret = lEval(nc,lCar(n));
	}
	lRootsRet(SP);
	return ret == NULL ? NULL : ret;
}

static lVal *lnfLetRaw(lClosure *c, lVal *v){
	const int SP = lRootsGet();
	lClosure *nc = lRootsClosurePush(lClosureNew(c));
	lVal *ret = lnfDo(nc,v);
	lRootsRet(SP);
	return ret;
}

static lClosure *getNextObject(lClosure *c){
	while(c != NULL){
		if(c->type == closureConstant){return NULL;}
		if(c->type == closureObject){return c;}
		c = c->parent;
	}
	return NULL;
}

static lVal *lnfClSelf(lClosure *c, lVal *v){
	c = getNextObject(c);
	for(int i=castToInt(lCar(v),0);i>0;i--){
		if(c == NULL){return NULL;}
		c = getNextObject(c->parent);
	}
	if(c == NULL){return NULL;}
	lVal *ret = lValAlloc();
	ret->type = ltObject;
	ret->vClosure = c;
	return ret;
}

static lVal *lnfResolve(lClosure *c, lVal *v){
	return lResolve(c,v);
}


/* Handler for [λ [...args] ...body] */
static lVal *lnfLambda(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	lVal *cdr = lCdr(v);
	if((v == NULL) || (car == NULL) || (cdr == NULL)){
		return NULL;
	}
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type           = ltLambda;
	ret->vClosure       = lClosureNew(c);
	ret->vClosure->doc  = lCons(car,lCar(cdr));
	ret->vClosure->text = lCons(NULL,NULL);
	ret->vClosure->text->vList.car = lnfvDo;
	ret->vClosure->text->vList.cdr = cdr;
	ret->vClosure->args = car;

	return ret;
}

/* Handler for [λ* [..args] docstring body] */
static lVal *lnfLambdaRaw(lClosure *c, lVal *v){
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type           = ltLambda;
	ret->vClosure       = lClosureNew(c);
	ret->vClosure->doc  = lCons(lCar(v),lCadr(v));
	ret->vClosure->text = lCaddr(v);
	ret->vClosure->args = lCar(v);

	return ret;
}

/* Handler for [δ [...args] ...body] */
static lVal *lnfDynamic(lClosure *c, lVal *v){
	lVal *ret = lnfLambda(c,v);
	if(ret == NULL){return NULL;}
	ret->type = ltDynamic;
	return ret;
}

/* Handler for [μ [...args] ...body] */
static lVal *lnfMacro(lClosure *c, lVal *v){
	lVal *ret = lnfLambda(c,v);
	if(ret == NULL){return NULL;}
	ret->type = ltMacro;
	return ret;
}

/* Handler for [ω ...body] */
static lVal *lnfObject(lClosure *c, lVal *v){
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type     = ltObject;
	ret->vClosure = lClosureNew(c);
	ret->vClosure->type = closureObject;
	lnfDo(ret->vClosure,v);
	return ret;
}

void lOperationsClosure(lClosure *c){
	lAddNativeFunc(c,"resolve",        "[sym]",         "Resolve SYM until it is no longer a symbol", lnfResolve);
	lAddNativeFunc(c,"cl",             "[i]",           "Return closure",                             lnfCl);
	lAddNativeFunc(c,"cl-lambda",      "[i]",           "Return closure as a lambda",                 lnfClLambda);
	lAddNativeFunc(c,"cl-text",        "[f]",           "Return closures text segment",               lnfClText);
	lAddNativeFunc(c,"cl-doc",         "[f]",           "Return documentation pair for F",            lnfClDoc);
	lAddNativeFunc(c,"cl-data",        "[f]",           "Return closures data segment",               lnfClData);
	lAddNativeFunc(c,"self",           "[n]",           "Return Nth closest object closure",          lnfClSelf);
	lAddNativeFunc(c,"symbol-table",   "[off len]",     "Return a list of len symbols defined, accessible from the current closure from offset off",lnfSymbolTable);
	lAddNativeFunc(c,"symbol-count",   "[]",            "Return a count of the symbols accessible from the current closure",lnfSymCount);

	lAddSpecialForm(c,"def",           "[sym val]",     "Define a new symbol SYM and link it to value VAL",                 lnfDef);
	lAddSpecialForm(c,"set!",          "[s v]",         "Bind a new value v to already defined symbol s",                   lnfSet);
	lAddSpecialForm(c,"let",           "[args ...body]","Create a new closure with args bound in which to evaluate ...body",lnfLet);
	lAddSpecialForm(c,"let*",          "[...body]",     "Run body wihtin a new closure",lnfLetRaw);

	lAddSpecialForm(c,"λ*",            "[args source body]", "Create a new, raw, lambda",             lnfLambdaRaw);

	lAddSpecialForm(c,"lambda fun λ", "[args ...body]", "Create a new lambda",                       lnfLambda);
	lAddSpecialForm(c,"dynamic dyn δ",   "[args ...body]", "New Dynamic scoped lambda",                 lnfDynamic);
	lAddSpecialForm(c,"macro μ",         "[args ...body]", "Create a new object",                       lnfMacro);
	lAddSpecialForm(c,"object ω",        "[...body]",      "Create a new object",                       lnfObject);
}
