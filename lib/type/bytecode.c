/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "bytecode.h"

#include "../exception.h"
#include "../allocation/symbol.h"
#include "../type/closure.h"
#include "../type/symbol.h"
#include "../collection/list.h"
#include "../misc/pf.h"

typedef enum lOpcode {
	lopNOP             =  0x0,
	lopRet             =  0x1,
	lopIntByte         =  0x2,
	lopIntAdd          =  0x3,
	lopDebugPrintStack =  0x4,
	lopPushLVal        =  0x5,
	lopMakeList        =  0x6,
	lopEval            =  0x7,
	lopApply           =  0x8,
	lopJmp             =  0x9,
	lopJt              =  0xA,
	lopJf              =  0xB,
	lopDup             =  0xC,
	lopDrop            =  0xD,
	lopDef             =  0xE,
	lopSet             =  0xF,
	lopGet             = 0x10,
	lopLambda          = 0x11,
	lopMacro           = 0x12,
	lopClosurePush     = 0x13,
	lopClosureEnter    = 0x14,
	lopLet             = 0x15,
	lopClosurePop      = 0x16,
	lopCall            = 0x17,
	lopTry             = 0x18,
	lopThrow           = 0x19,
	lopApplyDynamic    = 0x1A
} lOpcode;

int pushList(lVal **stack, int sp, lVal *args){
	if(!args){return sp;}
	sp = pushList(stack, sp, lCdr(args));
	stack[sp] = lCar(args);
	return sp + 1;
}

void printStack(lVal **stack, int sp, lClosure **cloStack, int csp){
	(void)cloStack;
	pf("-- Debug Output [SP:%u CSP:%u] -- %v\n", sp, csp);
	while(sp > 0){
		pf("%v\n",stack[--sp]);
	}
}

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

static lVal *lBytecodeReadOPVal(const lBytecodeOp **rip){
	const lBytecodeOp *ip = *rip;
	int i = *ip++;
	i = (i << 8) | *ip++;
	i = (i << 8) | *ip++;
	lVal *ret = lIndexVal(i);
	*rip = ip;
	return ret;
}

static lSymbol *lBytecodeReadOPSym(const lBytecodeOp **rip){
	const lBytecodeOp *ip = *rip;
	int i = *ip++;
	i = (i << 8) | *ip++;
	i = (i << 8) | *ip++;
	lSymbol *ret = lIndexSym(i);
	*rip = ip;
	return ret;
}

static int lBytecodeGetOffset16(const lBytecodeOp *ip){
	const int x = (ip[0] << 8) | ip[1];
	return (x < (1 << 15)) ? x : -((1<<16) - x);
}

lVal *lBytecodeEval(lClosure *callingClosure, lVal *args, const lBytecodeArray *ops){
	const lBytecodeOp *ip = ops->data;
	const int gcsp = lRootsGet();
	lVal *stack[VALUE_STACK_SIZE];
	lClosure *cloStack[CALL_STACK_SIZE];
	lClosure *c = lClosureNew(callingClosure);
	cloStack[0] = c;
	int sp  = pushList(stack, 0, args);
	int csp = 1;
	c->type = closureLet;

	while((ip >= ops->data) && (ip < ops->dataEnd)){
	switch(*ip){
	default:
		lExceptionThrowValClo(":unknown-opcode", "Stubmbled upon an unknown opcode", NULL, c);
		break;
	case lopNOP:
		ip++;
		break;
	case lopIntByte: {
		const i8 v = *++ip;
		stack[sp++] = lValInt(v);
		ip++;
		break;}
	case lopIntAdd: {
		if(sp < 2){lExceptionThrowValClo(":stack-underflow", "Underflowed the stack while returning", NULL, c);}
		const i64 a = castToInt(stack[sp-1],0);
		const i64 b = castToInt(stack[sp-2],0);
		stack[sp-2] = lValInt(a + b);
		sp--;
		ip++;
		break;}
	case lopDebugPrintStack:
		pf("Bytecode Debug stack:\n");
		printStack(stack, sp, cloStack, csp);
		ip++;
		break;
	case lopPushLVal: {
		ip++;
		stack[sp++] = lBytecodeReadOPVal(&ip);
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
		lVal *fun = RVP(stack[--sp]);
		lVal *cargs = lStackBuildList(stack, sp, len);
		sp -= len;
		ip++;
		stack[sp++] = lApply(c, cargs, fun, fun);
		break; }
	case lopDup:
		if(sp < 1){lExceptionThrowValClo(":stack-underflow", "Underflowed the stack while returning", NULL, c);}
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
		if(--sp < 0){lExceptionThrowValClo(":stack-underflow", "Underflowed the stack while returning", NULL, c);}
		ip++;
		break;
	case lopDef:
		ip++;
		lDefineClosureSym(c, lBytecodeReadOPSym(&ip), stack[sp - 1]);
		break;
	case lopSet:
		ip++;
		lSetClosureSym(c, lBytecodeReadOPSym(&ip), stack[--sp]);
		break;
	case lopGet:
		ip++;
		stack[sp++] = lGetClosureSym(c, lBytecodeReadOPSym(&ip));
		break;
	case lopLambda: {
		ip++;
		lVal *cName = lBytecodeReadOPVal(&ip);
		lVal *cArgs = lBytecodeReadOPVal(&ip);
		lVal *cDocs = lBytecodeReadOPVal(&ip);
		lVal *cBody = lBytecodeReadOPVal(&ip);
		stack[sp++] = lLambdaNew(c, cName, cArgs, cDocs, cBody);
		break;}
	case lopMacro: {
		ip++;
		lVal *cName = lBytecodeReadOPVal(&ip);
		lVal *cArgs = lBytecodeReadOPVal(&ip);
		lVal *cDocs = lBytecodeReadOPVal(&ip);
		lVal *cBody = lBytecodeReadOPVal(&ip);
		stack[sp++] = lLambdaNew(c, cName, cArgs, cDocs, cBody);
		if(stack[sp-1]){stack[sp-1]->type = ltMacro;}
		break;}
	case lopClosurePush:
		ip++;
		stack[sp++] = lValObject(c);
		break;
	case lopClosureEnter: {
		ip++;
		lVal *cObj = stack[--sp];
		if((cObj->type != ltLambda) && (cObj->type != ltObject)){lExceptionThrowValClo(":invalid-closure", "Error while trying to enter a closure", cObj, c);}
		cloStack[csp++] = c;
		c = cObj->vClosure;
		break; }
	case lopLet:
		ip++;
		cloStack[csp++] = c;
		c = lClosureNew(c);
		c->type = closureLet;
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
			lRootsRet(gcsp);
			if(sp < 1){lExceptionThrowValClo(":stack-underflow", "Underflowed the stack while returning", NULL, c);}
			return NULL;
		}
		break;
	case lopCall:
		c->ip = ip+3;
		c->sp = sp;
		cloStack[csp++] = c;
		c = lClosureNew(c);
		c->type = closureCall;
		ip += lBytecodeGetOffset16(ip+1);
		break;
	case lopTry:
		cloStack[csp++] = c;
		c = lClosureNew(c);
		c->type = closureTry;
		c->ip = ip + lBytecodeGetOffset16(ip+1);
		c->sp = sp;
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
			lRootsRet(gcsp);
			if(sp < 1){lExceptionThrowValClo(":stack-underflow", "Underflowed the stack while returning", NULL, c);}
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
			lRootsRet(gcsp);
			if(sp < 1){lExceptionThrowValClo(":stack-underflow", "Underflowed the stack while returning", NULL, c);}
			lExceptionThrowRaw(stack[--sp]);
			return NULL;
		}
		break;
	}}
	lExceptionThrowValClo(":expected-return", "The bytecode evaluator expected and explicit return operation", NULL, c);
	return NULL;
}

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

void lBytecodeArrayMark(const lBytecodeArray *v){
	for(const lBytecodeOp *c = v->data; c < v->dataEnd; c += lBytecodeOpLength(*c)){
		switch(*c){
		default: break;
		case lopPushLVal:
			lValGCMark(lIndexVal((c[1] << 16) | (c[2] << 8) | c[3]));
			break;
		}
	}

}
