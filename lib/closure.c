/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <ctype.h>

extern unsigned long long int bootstrap_image_len;
extern unsigned char bootstrap_image[];

lClosure *lClosureNew(lClosure *parent, closureType t) {
	lClosure *c = lClosureAllocRaw();
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
			continue;
		} else if(likely(n.type == ltSymbol)) {
			tmpc->data = lTreeInsert(tmpc->data, n.vSymbol, args);
		}
		return tmpc;
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

static lVal lAddNativeFuncRaw(const char *sym, const char *args, const char *doc, void *func, uint flags, u8 argCount){
	(void)args;
	lVal v = lValAlloc(ltNativeFunc, lNFuncAlloc());
	lSymbol *name = lSymS(sym);
	v.vNFunc->fp   = func;
	v.vNFunc->meta = lTreeInsert(NULL, symDocumentation, lValString(doc));
	v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symArguments, lValString(args));
	v.vNFunc->argCount = argCount;
	v.vNFunc->name = name;
	if(flags & NFUNC_FOLD){
		v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symFold, lValBool(true));
	}
	if(flags & NFUNC_PURE){
		v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symPure, lValBool(true));
	}

	v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symName, lValSymS(name));
	return v;
}
/* Native function argument count encoding:
 * Bit 0: has closure arg (C)
 * Bits 1-3: number of lVal args (V count)
 * Bit 4: is rest/variadic args (R)
 */
#define NFUNC_ARGC(hasC, vCount, hasR) ((hasC) | ((hasR) ? (1 << 4) : ((vCount) << 1)))

lVal lAddNativeFunc    (const char *sym, const char *args, const char *doc, lVal (*func)(), uint flags)                                    { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(0, 0, 0)); }
lVal lAddNativeFuncC   (const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *), uint flags)                          { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(1, 0, 0)); }
lVal lAddNativeFuncV   (const char *sym, const char *args, const char *doc, lVal (*func)(lVal), uint flags)                                { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(0, 1, 0)); }
lVal lAddNativeFuncCV  (const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal), uint flags)                    { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(1, 1, 0)); }
lVal lAddNativeFuncVV  (const char *sym, const char *args, const char *doc, lVal (*func)(lVal, lVal), uint flags)                          { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(0, 2, 0)); }
lVal lAddNativeFuncCVV (const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal, lVal), uint flags)              { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(1, 2, 0)); }
lVal lAddNativeFuncVVV (const char *sym, const char *args, const char *doc, lVal (*func)(lVal, lVal, lVal), uint flags)                    { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(0, 3, 0)); }
lVal lAddNativeFuncCVVV(const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal, lVal, lVal), uint flags)        { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(1, 3, 0)); }
lVal lAddNativeFuncVVVV(const char *sym, const char *args, const char *doc, lVal (*func)(lVal, lVal, lVal, lVal), uint flags)              { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(0, 4, 0)); }
lVal lAddNativeFuncCVVVV(const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal, lVal, lVal, lVal), uint flags) { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(1, 4, 0)); }
lVal lAddNativeFuncR   (const char *sym, const char *args, const char *doc, lVal (*func)(lVal), uint flags)                                { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(0, 0, 1)); }
lVal lAddNativeFuncCR  (const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *, lVal), uint flags)                    { return lAddNativeFuncRaw(sym, args, doc, func, flags, NFUNC_ARGC(1, 0, 1)); }

lClosure *findRoot (lVal v){
	switch(v.type){
	case ltEnvironment:
	case ltMacro:
	case ltLambda: {
		lClosure *c = v.vClosure;
		while(c->parent){
			c = c->parent;
		}
		return c;
	}
	default:
		return NULL;
	}
}

/* Create a new root closure with the stdlib */
lClosure *lNewRoot(){
	lVal imgVal = readImage(bootstrap_image, bootstrap_image_len, false);
	lClosure *c = findRoot(imgVal);
	lRedefineNativeFuncs(c);
	return c;
}

lClosure *lRedefineNativeFuncs(lClosure *c){
	for(uint i=0;i<lNFuncMax;i++){
		lNFunc *t = &lNFuncList[i];
		if(t == NULL){break;}
		lVal nf = lValAlloc(ltNativeFunc, t);
		lDefineClosureSym(c, t->name, nf);
	}
	lDefineTypeVars(c);
	lAddPlatformVars(c);

	return c;
}

/* Run fun with args  */
lVal lApply(lVal fun, lVal args){
	if(unlikely(fun.type != ltLambda)){
		return lValException(lSymTypeError, "Can't apply to following val", fun);
	}
	return lBytecodeEval(lClosureNewFunCall(args, fun), fun.vClosure->text);
}
