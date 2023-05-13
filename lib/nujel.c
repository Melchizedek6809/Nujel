/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <setjmp.h>
#include <stdlib.h>

extern u8 stdlib_no_data[];

jmp_buf exceptionTarget;
lVal *exceptionValue;
int exceptionTargetDepth = 0;

/* Initialize the allocator and symbol table, needs to be called before as
 * soon as possible, since most procedures depend on it.*/
void lInit(){
	lSymbolInit();
}

/* Super simple printer, not meant for production use, but only as a tool of last resort, for example when
 * we throw past the root exception handler.
 */
void simplePrintVal(lVal *v){
	typeswitch(v){
	default:
		fprintf(stderr, "#<not-printable-from-c %i> ", v->type);
		break;
	case ltEnvironment:
		fprintf(stderr, "#<env> ");
		break;
	case ltBytecodeArr:
		fprintf(stderr, "#<bc-arr> ");
		break;
	case ltBytecodeOp:
		fprintf(stderr, "#<bc-op %x> ", v->vBytecodeOp);
		break;
	case ltLambda:
		fprintf(stderr, "#<fn> ");
		break;
	case ltNativeFunc:
		fprintf(stderr, "#<NFn> ");
		break;
	case ltTree:
		fprintf(stderr, "#<tree> ");
		break;
	case ltNil:
		fprintf(stderr, "#nil ");
		break;
	case ltBool:
		fprintf(stderr, "%s ", v->vBool ? "#t" : "#f");
		break;
	case ltInt:
		fprintf(stderr, "%lli ", (long long int)v->vInt);
		break;
	case ltFloat:
		fprintf(stderr, "%f ", v->vFloat);
		break;
	case ltString:
		fprintf(stderr, "%s ", lStringData(v->vString));
		break;
	case ltKeyword:
		fprintf(stderr, ":"); // fall-through
	case ltSymbol:
		fprintf(stderr, "%s ", v->vSymbol->c);
		break;
	case ltPair:
		if(likely(v->vList)){
			simplePrintVal(v->vList->car);
			simplePrintVal(v->vList->cdr);
		}
		break;
	case ltArray:
		fprintf(stderr, "##(");
		for(int i=0;i<v->vArray->length;i++){
			simplePrintVal(v->vArray->data[i]);
		}
		fprintf(stderr, ") ");
	}
}

/* Cause an exception, passing V directly to the closest exception handler */
NORETURN void lExceptionThrowRaw(lVal *v){
	if(exceptionTargetDepth < 1){
		simplePrintVal(v);
		exit(201);
	}
	exceptionValue = v;
	longjmp(exceptionTarget, 1);
	while(1){}
}

/* Cause an exception, passing a list of SYMBOL, ERROR and V to the exception handler */
NORETURN void lExceptionThrowValClo(const char *symbol, const char *error, lVal *v, lClosure *c){
	lVal *l = lCons(lValLambda(c), NULL);
	l = lCons(v,l);
	l = lCons(lValString(error),l);
	l = lCons(lValKeyword(symbol),l);
	lExceptionThrowRaw(l);
}

/* Evaluate the Nujel Lambda expression and return the results */
lVal *lLambda(lClosure *c, lVal *args, lVal *lambda){
	return lBytecodeEval(lClosureNewFunCall(c, args, lambda), lambda->vClosure->text);
}

/* Run fun with args, evaluating args if necessary  */
lVal *lApply(lClosure *c, lVal *args, lVal *fun){
	switch(fun ? fun->type : ltNil){
	case ltLambda:     return lLambda(c,args,fun);
	case ltNativeFunc: return fun->vNFunc->fp(c,args);
	default:           lExceptionThrowValClo("type-error", "Can't apply to following val", fun, c);
	}
	return NULL;
}

/* Reads EXPR which should contain bytecode arrays and then evaluate them in C.
 * Mainly used for bootstrapping the stdlib and compiler out of precompiled .no
 * files. */
lClosure *lLoad(lClosure *c, const char *expr){
	lVal *v = lRead(c, expr);
	const int RSP = lRootsGet();
	for(lVal *n=v; n && n->type == ltPair; n = n->vList->cdr){
		c->args = n; // We need a reference to make sure that n won't be collected by the GC
		lVal *car = n->vList->car;
		if(unlikely((car == NULL) || (car->type != ltBytecodeArr))){
			lExceptionThrowValClo("load-error", "Can only load values of type :bytecode-arr", car, c);
		}else{
			lBytecodeEval(c, car->vBytecodeArr);
		}
	}
	c->args = NULL;
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

lVal *lCons(lVal *car, lVal *cdr){
	lVal *v = lValAlloc(ltPair);
	lPair *cons = lPairAllocRaw();
	cons->car = car;
	cons->cdr = cdr;
	v->vList = cons;
	return v;
}
