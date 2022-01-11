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
	lopNOP = 0,
	lopRet = 1,
	lopIntByte = 2,
	lopIntAdd = 3,
	lopDebugPrintStack = 4,
	lopPushLVal = 5,
	lopMakeList = 6,
	lopEval = 7,
	lopApply = 8,
	lopJmp = 9,
	lopJt = 10,
	lopDup = 11,
	lopDrop = 12,
	lopDef = 13,
	lopSet = 14,
	lopJf = 15,
} lOpcode;

int pushList(lVal **stack, int sp, lVal *args){
	if(!args){return sp;}
	sp = pushList(stack, sp, lCdr(args));
	stack[sp] = lCar(args);
	return sp + 1;
}

void printStack(lVal **stack, int sp){
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

lVal *lBytecodeEval(lClosure *c, lVal *args, const lBytecodeArray *ops){
	const lBytecodeOp *ip = ops->data;
	const int gcsp = lRootsGet();
	lVal *stack[BYTECODE_STACK_SIZE];
	int sp = pushList(stack, 0, args);

	while((ip >= ops->data) && (ip < ops->dataEnd)){
	switch(*ip){
	default:
		lExceptionThrowValClo(":unknown-opcode", "Stubmbled upon an unknown opcode", NULL, c);
		break;
	case lopNOP:
		ip++;
		break;
	case lopRet:
		lRootsRet(gcsp);
		if(sp < 1){lExceptionThrowValClo(":stack-underflow", "Underflowed the stack while returning", NULL, c);}
		return stack[--sp];
	case lopIntByte: {
		const i8 v = *++ip;
		stack[sp++] = lValInt(v);
		ip++;
		break;}
	case lopIntAdd: {
		const i64 a = castToInt(stack[sp-1],0);
		const i64 b = castToInt(stack[sp-2],0);
		stack[sp-2] = lValInt(a + b);
		sp--;
		ip++;
		break;}
	case lopDebugPrintStack:
		pf("Bytecode Debug stack:\n");
		printStack(stack, sp);
		ip++;
		break;
	case lopPushLVal: {
		int i = *++ip;
		i = (i << 8) | *++ip;
		i = (i << 8) | *++ip;
		ip++;
		stack[sp++] = lIndexVal(i);
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
	case lopDup:
		stack[sp] = stack[sp-1];
		sp++;
		ip++;
		break;
	case lopJmp: {
		int i = *++ip;
		i = (i << 8) | *++ip;
		ip++;
		ip += i;
		break; }
	case lopJt: {
		int i = *++ip;
		i = (i << 8) | *++ip;
		ip++;
		if(castToBool(stack[--sp])){ip += i;}
		break; }
	case lopJf: {
		int i = *++ip;
		i = (i << 8) | *++ip;
		ip++;
		if(!castToBool(stack[--sp])){ip += i;}
		break; }
	case lopDrop:
		sp--;
		ip++;
		break;
	case lopDef: {
		int i = *++ip;
		i = (i << 8) | *++ip;
		i = (i << 8) | *++ip;
		ip++;
		const lSymbol *s = lIndexSym(i);
		//pf("Define: %v[%i] := %v\n", lValSymS(s), i, stack[sp - 1]);
		lDefineClosureSym(c, s, stack[sp - 1]);
		break;}
	case lopSet: {
		int i = *++ip;
		i = (i << 8) | *++ip;
		i = (i << 8) | *++ip;
		ip++;
		const lSymbol *s = lIndexSym(i);
		lSetClosureSym(c, s, stack[sp - 1]);
		break;}
	}}
	lExceptionThrowValClo(":expected-return", "The bytecode evaluator expected and explicit return operation", NULL, c);
	return NULL;
}

static int lBytecodeOpLength(const lBytecodeOp op){
	switch(op){
	default:
		return 1;
	case lopMakeList:
	case lopIntByte:
		return 2;
	case lopJmp:
	case lopJf:
	case lopJt:
		return 3;
	case lopDef:
	case lopSet:
	case lopPushLVal:
		return 4;
	case lopApply:
		return 5;
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
