/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <ctype.h>
#include <string.h>

extern u8 stdlib_no_data[];

lClosure *lClosureNew(lClosure *parent, closureType t) {
	lClosure *c = lClosureAllocRaw();
	memset(c,0,sizeof(lClosure));
	c->parent = parent;
	c->type = t;
	return c;
}

lClosure *lClosureNewFunCall(lVal args, lVal lambda) {
	lClosure *tmpc = lClosureAllocRaw();
	tmpc->parent = lambda.vClosure;
	tmpc->type   = closureCall;
	tmpc->text   = lambda.vClosure->text;
	tmpc->ip     = tmpc->text->data;
	for (lVal n = lambda.vClosure->args; ; n = n.vList->cdr) {
		if (likely(n.type == ltPair)) {
			if(unlikely(args.type != ltPair)){
				tmpc->data = lTreeInsert(tmpc->data, n.vList->car.vSymbol, NIL);
			} else {
				tmpc->data = lTreeInsert(tmpc->data, n.vList->car.vSymbol, args.vList->car);
				args = args.vList->cdr;
			}
		} else if(likely(n.type == ltSymbol)) {
			tmpc->data = lTreeInsert(tmpc->data, n.vSymbol, args);
			break;
		} else {
			break;
		}
	}
	return tmpc;
}

lVal lGetClosureSym(lClosure *c, const lSymbol *s){
	for (const lClosure *cc = c; cc; cc = cc->parent) {
		const lTree *t = cc->data;
		while(t){
			if(s == t->key){
				return t->value;
			}
			t = s > t->key ? t->right : t->left;
		}
	}
	return lValException(lSymUnboundVariable, "Can't resolve symbol", lValSymS(s));
}

/* Bind the value V to the Symbol S in the closure C, defining it if necessary */
void lDefineClosureSym(lClosure *c, const lSymbol *s, lVal v){
	c->data = lTreeInsert(c->data, s, v);
}

/* Set the value bound to S in C to V, if it has already been bound */
bool lSetClosureSym(lClosure *c, const lSymbol *s, lVal v){
	for (lClosure *cc = c; cc; cc = cc->parent) {
		lTree *t = cc->data;
		while(t){
			if(t->key == s){
				t->value = v;
				return true;
			}
			t = s > t->key ? t->right : t->left;
		}
	}
	return false;
}

/* Turn STR into a symbol, and bind VAL to it within C */
void lDefineVal(lClosure *c, const char *str, lVal val){
	lDefineClosureSym(c, lSymS(str), val);
}

static lVal lAddNativeFuncRaw(lClosure *c, const char *sym, const char *args, const char *doc, void *func, uint flags, u8 argCount){
	lVal v = lValAlloc(ltNativeFunc, lNFuncAlloc());
	lSymbol *name = lSymS(sym);
	v.vNFunc->fp   = func;
	v.vNFunc->args = lCar(lRead(args, strlen(args)));
	v.vNFunc->meta = lTreeInsert(NULL, symDocumentation, lValString(doc));
	v.vNFunc->argCount = argCount;
	v.vNFunc->name = name;
	if(flags & NFUNC_FOLD){
		v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symFold, lValBool(true));
	}
	if(flags & NFUNC_PURE){
		v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symPure, lValBool(true));
	}

	v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symName, lValSymS(name));
	lDefineClosureSym(c, name, v);
	return v;
}
lVal lAddNativeFunc(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 0);
}
lVal lAddNativeFuncC(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 1);
}
lVal lAddNativeFuncV(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 1 << 1);
}
lVal lAddNativeFuncCV(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 1 | (1 << 1));
}
lVal lAddNativeFuncVV(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lVal, lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 2 << 1);
}
lVal lAddNativeFuncCVV(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal, lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 1 | (2 << 1));
}
lVal lAddNativeFuncVVV(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lVal, lVal, lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 3 << 1);
}
lVal lAddNativeFuncCVVV(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal, lVal, lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 1 | (3 << 1));
}
lVal lAddNativeFuncVVVV(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lVal, lVal, lVal, lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 4 << 1);
}
lVal lAddNativeFuncCVVVV(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal, lVal, lVal, lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 1 | (4 << 1));
}
lVal lAddNativeFuncR(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 1 << 4);
}
lVal lAddNativeFuncCR(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal), uint flags){
	return lAddNativeFuncRaw(c, sym, args, doc, func, flags, 1 | (1 << 4));
}

/* Create a new Lambda Value */
lVal lLambdaNew(lClosure *parent, lVal args, lVal body){
	lVal ret = lValAlloc(ltLambda, lClosureNew(parent, closureDefault));
	ret.vClosure->args = args;
	reqBytecodeArray(body);
	ret.vClosure->text = body.vBytecodeArr;
	ret.vClosure->ip   = ret.vClosure->text->data;
	return ret;
}

void lClosureSetMeta(lClosure *c, lVal doc){
	if(unlikely(doc.type != ltTree)){
		return;
	}
	lTree *t = doc.vTree->root;
	c->meta = (t && t->flags & TREE_IMMUTABLE) ? lTreeDup(t) : t;
}

/* Add all the essential Native Functions to closure c */
lClosure *lInitRootClosure(){
	lClosure *c = lClosureAllocRaw();
	c->type = closureRoot;
	lTypesInit(c);
	lOperationsArithmetic(c);
	lOperationsBuffer(c);
	lOperationsArray(c);
	lOperationsCore(c);
	lOperationsSpecial(c);
	lOperationsTree(c);
	lOperationsImage(c);
	lOperationsBytecode();
	lOperationsString();
	lAddPlatformVars(c);
	lDefineVal(c,"exports",  lValTree(NULL));
	lDefineVal(c,"*module*", lValKeyword("core"));
	return c;
}

/* Create a new root closure with the stdlib */
lClosure *lNewRoot(){
	return lLoad(lInitRootClosure(), (const char *)stdlib_no_data);
}

static lClosure *findRootRec(lClosure *v){
	if(v->type == closureRoot){
		return v;
	} else {
		return findRootRec(v->parent);
	}
}

lClosure *findRoot (lVal v){
	switch(v.type){
	case ltEnvironment:
	case ltMacro:
	case ltLambda:
		return findRootRec(v.vClosure);
	default:
		return NULL;
	}
}

lClosure *lRedefineNativeFuncs(lClosure *c){
	for(uint i=0;i<lNFuncMax;i++){
		lNFunc *t = &lNFuncList[i];
		if(t == NULL){break;}
		lVal nf = lValAlloc(ltNativeFunc, t);
		lDefineClosureSym(c, t->name, nf);
	}
	return c;
}

/* Run fun with args  */
lVal lApply(lVal fun, lVal args){
	if(unlikely(fun.type != ltLambda)){
		return lValException(lSymTypeError, "Can't apply to following val", fun);
	}
	return lBytecodeEval(lClosureNewFunCall(args, fun), fun.vClosure->text);
}
