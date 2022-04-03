/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "bytecode.h"

#include "../exception.h"
#include "../allocation/symbol.h"
#include "../type/closure.h"
#include "../type/symbol.h"
#include "../collection/list.h"
#include "../misc/pf.h"

#include <string.h>

/* Push the list args onto the stack, return the new SP */
int pushList(lVal **stack, int sp, lVal *args){
	if(!args){return sp;}
	sp = pushList(stack, sp, lCdr(args));
	stack[sp] = lCar(args);
	return sp + 1;
}

/* Print the contents of the stack, mostly used for debuggin */
void printStack(lVal **stack, int sp, lClosure **cloStack, int csp){
	(void)cloStack;
	pf("-- Debug Output [SP:%u CSP:%u] -- %v\n", sp, csp);
	while(sp > 0){
		pf("%v\n",stack[--sp]);
	}
}

/* Build a list of length len in stack starting at sp */
static lVal *lStackBuildList(lVal **stack, int sp, int len){
	lVal *ret, *t = NULL;
	ret = RVP(lCons(NULL,NULL));
	for(int i = len; i > 0; i--){
		if(t == NULL){
			ret->vList.car = stack[sp - i];
			t = ret;
		}else{
			t->vList.cdr = lCons(stack[sp - i],NULL);
			t = t->vList.cdr;
		}
	}
	return ret;
}

/* Read a value referenced at IP and store it in RET, retuns the new IP */
static const lBytecodeOp *lBytecodeReadOPVal(const lBytecodeOp *ip, lVal **ret){
	int i = *ip++;
	i = (i << 8) | *ip++;
	i = (i << 8) | *ip++;
	*ret = lIndexVal(i);
	return ip;
}

/* Read a symbol referenced at IP and store it in RET, retuns the new IP */
static const lBytecodeOp *lBytecodeReadOPSym(const lBytecodeOp *ip, lSymbol **ret){
	int i = *ip++;
	i = (i << 8) | *ip++;
	i = (i << 8) | *ip++;
	*ret = lIndexSym(i);
	return ip;
}

/* Read an encoded signed 16-bit offset at ip */
static int lBytecodeGetOffset16(const lBytecodeOp *ip){
	const int x = (ip[0] << 8) | ip[1];
	return (x < (1 << 15)) ? x : -((1<<16) - x);
}

/* Evaluate ops within callingClosure after pushing args on the stack */
lVal *lBytecodeEval(lClosure *callingClosure, lVal *args, const lBytecodeArray *ops){
	jmp_buf oldExceptionTarget;
	const lBytecodeOp *ip;
	int rootsStack[ROOT_STACK_SIZE];
	rootsStack[0] = lRootsGet();
	int rsp = 1;

	lVal *stack[VALUE_STACK_SIZE];
	lClosure *cloStack[CALL_STACK_SIZE];
	lClosure *c = lClosureNew(callingClosure);
	volatile int csp = 1;
	int sp;
	int exceptionCount = 0;
	c->type = closureLet;
	memset(stack, 0, sizeof(stack));
	memset(cloStack, 0, sizeof(cloStack));
	lRootsValStackPush(stack);
	lRootsCallStackPush(cloStack);
	cloStack[0] = c;

	memcpy(oldExceptionTarget,exceptionTarget,sizeof(jmp_buf));
	exceptionTargetDepth++;
	const int setjmpRet = setjmp(exceptionTarget);
	if(setjmpRet){
		while((csp >= 0) && (c->type != closureTry)){
			c = cloStack[--csp];
		}
		if((csp >= 0) && (++exceptionCount < 100000) && (c->type == closureTry)){
			ip = c->ip;
			sp = c->sp;
			stack[sp++] = exceptionValue;
			c = cloStack[--csp];
		}else{
			memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
			lExceptionThrowRaw(exceptionValue);
			return NULL;
		}
	}else{
		ip = ops->data;
		sp = pushList(stack, 0, args);
		csp = 1;
	}

	while((ip >= ops->data) && (ip < ops->dataEnd)){
	switch(*ip){
	default:
		lExceptionThrowValClo("unknown-opcode", "Stubmbled upon an unknown opcode", NULL, c);
		break;
	case lopNOP:
		ip++;
		break;
	case lopIntByte: {
		const i8 v = *++ip;
		stack[sp++] = lValInt(v);
		ip++;
		break;}
	case lopIntAdd:
		if(sp < 2){lExceptionThrowValClo("stack-underflow", "A stack underflow occured", NULL, c);}
		stack[sp-2] = lValInt(castToInt(stack[sp-2],0) + castToInt(stack[sp-1],0));
		sp--;
		ip++;
		break;
	case lopLessPred:
		if(sp < 2){lExceptionThrowValClo("stack-underflow", "A stack underflow occured", NULL, c);}
		stack[sp-2] = lValBool(lValCompare(stack[sp-2], stack[sp-1]) == -1);
		sp--;
		ip++;
		break;
	case lopLessEqPred:
		if(sp < 2){lExceptionThrowValClo("stack-underflow", "A stack underflow occured", NULL, c);}
		stack[sp-2] = lValBool(lValCompare(stack[sp-2],stack[sp-1]) <= 0);
		sp--;
		ip++;
		break;
	case lopEqualPred:
		if(sp < 2){lExceptionThrowValClo("stack-underflow", "A stack underflow occured", NULL, c);}
		stack[sp-2] = lValBool(lValCompare(stack[sp-2],stack[sp-1]) == 0);
		sp--;
		ip++;
		break;
	case lopGreaterEqPred: {
		if(sp < 2){lExceptionThrowValClo("stack-underflow", "A stack underflow occured", NULL, c);}
		const int cmp = lValCompare(stack[sp-2],stack[sp-1]);
		stack[sp-2] = lValBool((cmp == 1) || (cmp == 0));
		sp--;
		ip++;
		break; }
	case lopGreaterPred:
		if(sp < 2){lExceptionThrowValClo("stack-underflow", "A stack underflow occured", NULL, c);}
		stack[sp-2] = lValBool(lValCompare(stack[sp-2],stack[sp-1]) == 1);
		sp--;
		ip++;
		break;
	case lopDebugPrintStack:
		pf("Bytecode Debug stack:\n");
		printStack(stack, sp, cloStack, csp);
		ip++;
		break;
	case lopPushLVal:
		ip = lBytecodeReadOPVal(ip+1, &stack[sp++]);
		break;
	case lopPushSymbol: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		stack[sp++] = lValSymS(sym);
		break;}
	case lopMakeList: {
		const int len = *++ip;
		lVal *ret = lStackBuildList(stack, sp, len);
		sp -= len;
		stack[sp++] = ret;
		ip++;
		break;}
	case lopEval:
		stack[sp-1] = lEval(c,stack[sp-1]);
		ip++;
		break;
	case lopApply: {
		const int len = *++ip;
		lVal *cargs = lStackBuildList(stack, sp, len);
		sp -= len;
		ip++;
		lVal *fun = lIndexVal((ip[0] << 16) | (ip[1] << 8) | ip[2]);
		stack[sp++] = lApply(c, cargs, fun, fun);
		ip += 3;
		break; }
	case lopApplyDynamic: {
		const int len = *++ip;
		lVal *cargs = lStackBuildList(stack, sp, len);
		sp -= len;
		lVal *fun = RVP(stack[--sp]);
		ip++;
		stack[sp++] = lApply(c, cargs, fun, fun);
		break; }
	case lopDup:
		if(sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed the stack while returning", NULL, c);}
		stack[sp] = stack[sp-1];
		sp++;
		ip++;
		break;
	case lopJmp:
		ip += lBytecodeGetOffset16(ip+1);
		break;
	case lopJt:
		ip +=  castToBool(stack[--sp]) ? lBytecodeGetOffset16(ip+1) : 3;
		break;
	case lopJf:
		ip += !castToBool(stack[--sp]) ? lBytecodeGetOffset16(ip+1) : 3;
		break;
	case lopDrop:
		if(--sp < 0){lExceptionThrowValClo("stack-underflow", "Underflowed the stack while returning", NULL, c);}
		ip++;
		break;
	case lopDef: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		lDefineClosureSym(c, sym, stack[sp - 1]);
		break; }
	case lopSet: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		lSetClosureSym(c, sym, stack[sp - 1]);
		break; }
	case lopGet: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		stack[sp++] = lGetClosureSym(c, sym);
		break; }
	case lopLambda: {
		lVal *cName, *cArgs, *cDocs, *cBody;
		ip = lBytecodeReadOPVal(ip+1, &cName);
		ip = lBytecodeReadOPVal(ip, &cArgs);
		ip = lBytecodeReadOPVal(ip, &cDocs);
		ip = lBytecodeReadOPVal(ip, &cBody);
		stack[sp++] = lLambdaNew(c, cName, cArgs, cDocs, cBody);
		break;}
	case lopMacro: {
		lVal *cName, *cArgs, *cDocs, *cBody;
		ip = lBytecodeReadOPVal(ip+1, &cName);
		ip = lBytecodeReadOPVal(ip, &cArgs);
		ip = lBytecodeReadOPVal(ip, &cDocs);
		ip = lBytecodeReadOPVal(ip, &cBody);
		stack[sp++] = lLambdaNew(c, cName, cArgs, cDocs, cBody);
		if(stack[sp-1]){
			stack[sp-1]->type = ltMacro;
		}
		break; }
	case lopRootsPush:
		ip++;
		rootsStack[rsp++] = lRootsGet();
		break;
	case lopRootsPop:
		ip++;
		if(rsp == 0){
			lExceptionThrowValClo("stack-underflow", "Poped one too many times off the roots stack", NULL, c);
		}
		lRootsRet(rootsStack[--rsp]);
		break;
	case lopRootsPeek:
		ip++;
		lRootsRet(rootsStack[rsp - 1]);
		break;
	case lopClosurePush:
		ip++;
		stack[sp++] = lValObject(c);
		break;
	case lopClosureEnter: {
		ip++;
		lVal *cObj = stack[--sp];
		if((cObj->type != ltLambda) && (cObj->type != ltObject)){
			lExceptionThrowValClo("invalid-closure", "Error while trying to enter a closure", cObj, c);
		}
		cloStack[csp++] = c;
		c = cObj->vClosure;
		cloStack[csp] = c;
		break; }
	case lopLet:
		ip++;
		cloStack[csp++] = c;
		c = lClosureNew(c);
		c->type = closureLet;
		cloStack[csp] = c;
		break;
	case lopClosurePop:
		ip++;
		while(csp > 0){
			lClosure *nclo = cloStack[--csp];
			if(nclo->type == closureTry){continue;}
			c = nclo;
			break;
		}
		if(csp == 0){
			lRootsRet(rootsStack[0]);
			if(sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed the stack while returning", NULL, c);}
			return NULL;
		}
		break;
	case lopCall:
		c->ip = ip+3;
		c->sp = sp;
		cloStack[csp++] = c;
		c = lClosureNew(c);
		c->type = closureCall;
		cloStack[csp] = c;
		ip += lBytecodeGetOffset16(ip+1);
		break;
	case lopTry:
		cloStack[csp++] = c;
		c = lClosureNew(c);
		c->type = closureTry;
		c->ip = ip + lBytecodeGetOffset16(ip+1);
		c->sp = sp;
		cloStack[csp] = c;
		ip+=3;
		break;
	case lopRet:
		while(csp > 0){
			lVal *rv = stack[sp-1];
			lClosure *nclo = cloStack[--csp];
			if(csp == 0){break;}
			if((nclo->type == closureTry)
			|| (nclo->type == closureLet)){
				continue;
			}
			ip = nclo->ip;
			sp = nclo->sp;
			stack[sp++] = rv;
			break;
		}
		if(csp == 0){
			lRootsRet(rootsStack[0]);
			if(sp < 1){
				lExceptionThrowValClo("stack-underflow", "Underflowed the stack while returning", NULL, c);
				return NULL;
			}
			memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
			exceptionTargetDepth--;
			return stack[--sp];
		}
		break;
	case lopThrow:
		ip++;
		while(csp > 0){
			lVal *rv = stack[sp-1];
			lClosure *nclo = cloStack[--csp];
			if(!nclo || csp == 0){break;}
			if(nclo->type != closureTry){continue;}
			ip = nclo->ip;
			sp = nclo->sp;
			stack[sp++] = rv;
			break;
		}
		if(csp == 0){
			lRootsRet(rootsStack[0]);
			if(sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed the stack while returning", NULL, c);}
			lExceptionThrowRaw(stack[--sp]);
			return NULL;
		}
		break;
	}}
	lExceptionThrowValClo("expected-return", "The bytecode evaluator expected and explicit return operation", NULL, c);
	return NULL;
}

/* Return the overall length of opcode op */
static int lBytecodeOpLength(const lBytecodeOp op){
	switch(op){
	default:
		return 1;
	case lopMakeList:
	case lopApplyDynamic:
	case lopIntByte:
		return 2;
	case lopCall:
	case lopTry:
	case lopJmp:
	case lopJf:
	case lopJt:
		return 3;
	case lopPushSymbol:
	case lopDef:
	case lopSet:
	case lopGet:
	case lopPushLVal:
		return 4;
	case lopApply:
		return 5;
	case lopLambda:
	case lopMacro:
		return 4*3+1;
	}
}

/* Mark all objects references within v, should only be called from the GC */
void lBytecodeArrayMark(const lBytecodeArray *v){
	for(const lBytecodeOp *c = v->data; c < v->dataEnd; c += lBytecodeOpLength(*c)){
		switch(*c){
		default: break;
		case lopPushSymbol:
		case lopDef:
		case lopGet:
		case lopSet:
			if(&c[3] >= v->dataEnd){break;}
			lSymbolGCMark(lIndexSym((c[1] << 16) | (c[2] << 8) | c[3]));
			break;
		case lopApply:
			if(&c[4] >= v->dataEnd){break;}
			lValGCMark(lIndexVal((c[ 2] << 16) | (c[ 3] << 8) | c[ 4]));
			break;
		case lopLambda:
		case lopMacro:
			if(&c[12] >= v->dataEnd){break;}
			lValGCMark(lIndexVal((c[ 1] << 16) | (c[ 2] << 8) | c[ 3]));
			lValGCMark(lIndexVal((c[ 4] << 16) | (c[ 5] << 8) | c[ 6]));
			lValGCMark(lIndexVal((c[ 7] << 16) | (c[ 8] << 8) | c[ 9]));
			lValGCMark(lIndexVal((c[10] << 16) | (c[11] << 8) | c[12]));
			break;
		case lopPushLVal:
			if(&c[3] >= v->dataEnd){break;}
			lValGCMark(lIndexVal((c[1] << 16) | (c[2] << 8) | c[3]));
			break;
		}
	}
}

/* Links a bytecode array, mostly used after serializing and deserializing
 * a function */
void lBytecodeLink(lClosure *clo){
	lBytecodeArray *v = &clo->text->vBytecodeArr;
	for(lBytecodeOp *c = v->data; c < v->dataEnd; c += lBytecodeOpLength(*c)){
		switch(*c){
		default: break;
		case lopApply: {
			if(&c[4] >= v->dataEnd){break;}
			lVal *raw = lIndexVal((c[2] << 16) | (c[3] << 8) | c[4]);
			if((!raw) || (raw->type != ltSymbol)){break;}
			lVal *n = lGetClosureSym(clo, raw->vSymbol);
			if(n == raw){break;}
			int i = lValIndex(n);
			c[2] = (i >> 16) & 0xFF;
			c[3] = (i >>  8) & 0xFF;
			c[4] = (i      ) & 0xFF;
			break; }
		case lopPushLVal:
			if(&c[3] >= v->dataEnd){break;}
			lVal *raw = lIndexVal((c[1] << 16) | (c[2] << 8) | c[3]);
			if(!raw || (raw->type != ltSymbol)){break;}
			lVal *n = lGetClosureSym(clo, raw->vSymbol);
			if(n == raw){break;}
			int i = lValIndex(n);
			c[1] = (i >> 16) & 0xFF;
			c[2] = (i >>  8) & 0xFF;
			c[3] = (i      ) & 0xFF;
			break;
		}
	}
}
