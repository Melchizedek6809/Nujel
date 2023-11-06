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

/* Run fun with args  */
lVal lApply(lVal fun, lVal args){
	if(unlikely(fun.type != ltLambda)){
		return lValException(lSymTypeError, "Can't apply to following val", fun);
	}
	return lBytecodeEval(lClosureNewFunCall(args, fun), fun.vClosure->text);
}

/* Reads EXPR which should contain bytecode arrays and then evaluate them in C.
 * Mainly used for bootstrapping the stdlib and compiler out of precompiled .no
 * files. */
lClosure *lLoad(lClosure *c, const char *expr){
	lVal v = lRead(expr);
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

/* Add all the essential Native Functions to closure c */
static void lInitRootClosure(lClosure *c){
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
}

/* Create a new root closure with the stdlib */
lClosure *lNewRoot(){
	lClosure *c = lClosureAllocRaw();
	lInitRootClosure(c);
	return lLoad(c, (const char *)stdlib_no_data);
}
