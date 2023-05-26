/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <ctype.h>
#include <string.h>

lClosure *lClosureNew(lClosure *parent, closureType t) {
	lClosure *c = lClosureAllocRaw();
	*c = (lClosure){0};
	c->parent = parent;
	c->type = t;
	return c;
}

lClosure *lClosureNewFunCall(lClosure *parent, lVal args, lVal lambda) {
	lClosure *tmpc = lClosureAllocRaw();
	tmpc->parent = lambda.vClosure;
	tmpc->type   = closureCall;
	tmpc->text   = lambda.vClosure->text;
	tmpc->caller = parent;
	tmpc->ip     = tmpc->text->data;
	for (lVal n = lambda.vClosure->args; ; n = n.vList->cdr) {
		if (likely(n.type == ltPair)) {
			if(unlikely(args.type != ltPair)){
				lDefineClosureSym(tmpc, n.vList->car.vSymbol, NIL);
			} else {
				lDefineClosureSym(tmpc, n.vList->car.vSymbol, args.vList->car);
				args = args.vList->cdr;
			}
		} else if(likely(n.type == ltSymbol)) {
			lDefineClosureSym(tmpc, n.vSymbol, args);
			break;
		} else {
			return tmpc;
		}
	}
	return tmpc;
}

/* Define the NFunc LNF in C with all the space separated symbols in SYM */
lVal lDefineAliased(lClosure *c, lVal lNF, const char *sym){
	const char *cur = sym;
	if(unlikely(lNF.type != ltNativeFunc)){
		lExceptionThrowValClo(":invalid-alias-definition","Only native functions and special forms can be defined with an alias",lNF, c);
	}
	bool nameSet = false;

	// Run at most 64 times, just a precaution
	for(int i=0;i<64;i++){
		uint len;
		for(len=0;len < sizeof(lSymbol);len++){ // Find the end of the current token, either space or 0
			if(cur[len] == 0)        {break;}
			if(isspace((u8)cur[len])){break;}
		}
		lSymbol *name = lSymSL(cur,len);
		lDefineClosureSym(c,name,lNF);
		if(!nameSet){
			lNF.vNFunc->meta = lTreeInsert(lNF.vNFunc->meta, symName, lValSymS(name));
			nameSet = true;
		}
		for(;len<32;len++){ // Advance to the next non whitespace character
			if(cur[len] == 0)    {return lNF;} // Or return if we reached the final 0 byte
			if(!isspace((u8)cur[len])){break;}
		}
		cur += len;
	}
	exit(125);
	return NIL;
}

/* Return TRUE if C contains a binding for S, storing the value in V */
bool lHasClosureSym(lClosure *c, const lSymbol *s, lVal *v){
	bool found = false;
	while(c != NULL){
		lVal t = lTreeGet(c->data,s,&found);
		if(found){
			if(v != NULL){
				*v = t;
			}
			return true;
		}
		c = c->parent;
	}
	return false;
}

lVal lGetClosureSym(lClosure *c, const lSymbol *s){
	for (lClosure *cc = c; cc; cc = cc->parent) {
		lVal ret;
		if(lTreeHas(cc->data,s,&ret)){
			return ret;
		}
	}
	lExceptionThrowValClo("unbound-variable","Can't resolve symbol", lValSymS(s), c);
	return NIL;
}

/* Bind the value V to the Symbol S in the closure C, defining it if necessary */
void lDefineClosureSym(lClosure *c, const lSymbol *s, lVal v){
	c->data = lTreeInsert(c->data, s, v);
}

/* Set the value bound to S in C to V, if it has already been bound */
bool lSetClosureSym(lClosure *c, const lSymbol *s, lVal v){
	if(unlikely(c == NULL)){return false;}
	bool found = false;
	lTreeSet(c->data, s, v, &found);
	if(found){
		return true;
	}
	return lSetClosureSym(c->parent, s, v);
}

/* Turn STR into a symbol, and bind VAL to it within C */
void lDefineVal(lClosure *c, const char *str, lVal val){
	lDefineClosureSym(c, lSymS(str), val);
}

/* Add a NFunc to closure C, should only be used during root closure creation */
lVal lAddNativeFunc(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal)){
	lVal v = lValAlloc(ltNativeFunc, lNFuncAlloc());
	v.vNFunc->fp   = func;
	v.vNFunc->meta = lTreeInsert(NULL, symDocumentation, lValString(doc));
	v.vNFunc->args = lCar(lRead(c, args));
	return lDefineAliased(c,v,sym);
}

lVal lAddNativeFuncFold(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal)){
	lVal v = lAddNativeFunc(c, sym, args, doc, func);
	v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symFold, lValBool(true));
	return v;
}

lVal lAddNativeFuncPure(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal)){
	lVal v = lAddNativeFunc(c, sym, args, doc, func);
	v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symPure, lValBool(true));
	return v;
}

lVal lAddNativeFuncPureFold(lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal)){
	lVal v = lAddNativeFuncFold(c, sym, args, doc, func);
	v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symPure, lValBool(true));
	return v;
}

/* Create a new Lambda Value */
lVal lLambdaNew(lClosure *parent, lVal args, lVal body){
	lVal ret = lValAlloc(ltLambda, lClosureNew(parent, closureDefault));
	ret.vClosure->args = args;
	ret.vClosure->text = requireBytecodeArray(parent, body);
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
