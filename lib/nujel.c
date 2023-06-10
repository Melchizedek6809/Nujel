/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <stdlib.h>

extern u8 stdlib_no_data[];

lVal NIL;

/* Initialize the allocator and symbol table, needs to be called before as
 * soon as possible, since most procedures depend on it.*/
void lInit(){
	lSymbolInit();
}

void simplePrintTree(lTree *t){
	if(!t){return;}
	simplePrintTree(t->left);
	if(t->key){
		fprintf(stderr, ":%s ", t->key->c);
		simplePrintVal(t->value);
	}
	simplePrintTree(t->right);
}

/* Super simple printer, not meant for production use, but only as a tool of last resort, for example when
 * we throw past the root exception handler.
 */
void simplePrintVal(lVal v){
	switch(v.type){
	default:
		fprintf(stderr, "#<not-printable-from-c %i> ", v.type);
		break;
	case ltEnvironment:
		fprintf(stderr, "#<env> ");
		break;
	case ltBytecodeArr:
		fprintf(stderr, "#<bc-arr> ");
		break;
	case ltLambda:
		fprintf(stderr, "#<fn> ");
		break;
	case ltNativeFunc:
		fprintf(stderr, "#<NFn> ");
		break;
	case ltTree:
		fprintf(stderr, " { ");
		simplePrintTree(v.vTree->root);
		fprintf(stderr, " } ");
		break;
	case ltNil:
		fprintf(stderr, "#nil ");
		break;
	case ltBool:
		fprintf(stderr, "%s ", v.vBool ? "#t" : "#f");
		break;
	case ltInt:
		fprintf(stderr, "%lli ", (long long int)v.vInt);
		break;
	case ltFloat:
		fprintf(stderr, "%f ", v.vFloat);
		break;
	case ltString:
		fprintf(stderr, "%s ", (const char *)lBufferData(v.vString));
		break;
	case ltKeyword:
		fprintf(stderr, ":"); // fall-through
	case ltSymbol:
		fprintf(stderr, "%s ", v.vSymbol->c);
		break;
	case ltPair:
		if(likely(v.vList)){
			simplePrintVal(v.vList->car);
			simplePrintVal(v.vList->cdr);
		}
		break;
	case ltArray:
		fprintf(stderr, "##(");
		for(int i=0;i<v.vArray->length;i++){
			simplePrintVal(v.vArray->data[i]);
		}
		fprintf(stderr, ") ");
	}
}

lVal lValException(const char *symbol, const char *error, lVal v) {
	lVal l = lCons(v, NIL);
	l = lCons(lValString(error),l);
	l = lCons(lValKeyword(symbol),l);
	l.type = ltException;
	return l;
}

/* Evaluate the Nujel Lambda expression and return the results */
lVal lLambda(lVal args, lVal lambda){
	return lBytecodeEval(lClosureNewFunCall(args, lambda), lambda.vClosure->text);
}

/* Run fun with args, evaluating args if necessary  */
lVal lApply(lClosure *c, lVal args, lVal fun){
	switch(fun.type){
	case ltMacro:
	case ltLambda:     return lLambda(args,fun);
	case ltNativeFunc: return fun.vNFunc->fp(c,args);
	default:           return lValException("type-error", "Can't apply to following val", fun);
	}
	return NIL;
}

/* Reads EXPR which should contain bytecode arrays and then evaluate them in C.
 * Mainly used for bootstrapping the stdlib and compiler out of precompiled .no
 * files. */
lClosure *lLoad(lClosure *c, const char *expr){
	lVal v = lRead(c, expr);
	const int RSP = lRootsGet();
	for(lVal n=v; n.type == ltPair; n = n.vList->cdr){
		c->args = n; // We need a reference to make sure that n won't be collected by the GC
		lVal car = n.vList->car;
		if(likely(car.type == ltBytecodeArr)){
			lBytecodeEval(c, car.vBytecodeArr);
		}
	}
	c->args = NIL;
	lRootsRet(RSP);
	return c;
}

void lOperationsBase(lClosure *c){
	lOperationsArithmetic(c);
	lOperationsBuffer(c);
	lOperationsArray(c);
	lOperationsBytecode(c);
	lOperationsCore(c);
	lOperationsSpecial(c);
	lOperationsString(c);
	lOperationsTree(c);
}

/* Create a new root closure with the stdlib */
lClosure *lNewRoot(){
	lClosure *c = lClosureAllocRaw();
	c->type = closureRoot;
	lOperationsBase(c);
	lAddPlatformVars(c);
	lDefineVal(c,"exports",  lValTree(NULL));
	lDefineVal(c,"*module*", lValKeyword("core"));
	return lLoad(c, (const char *)stdlib_no_data);
}
