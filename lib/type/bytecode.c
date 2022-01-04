/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "bytecode.h"

#include "../exception.h"
#include "../collection/list.h"
#include "../misc/pf.h"

typedef enum lOpcode {
	lopNOP = 0,
	lopRet = 1,
	lopIntByte = 2,
	loplVal = 2,
	lopIntAdd = 3,
	lopDebugPrintStack = 4,
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

lVal *lBytecodeEval(lClosure *c, lVal *args, const lBytecodeArray *ops){
	const lBytecodeOp *ip = ops->data;
	const int gcsp = lRootsGet();
	lVal *stack[BYTECODE_STACK_SIZE];
	int sp = pushList(stack, 0, args);

	while((ip >= ops->data) && (ip < ops->dataEnd)){
	switch(*ip){
	default:
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
	}}
	lExceptionThrowValClo(":expected-return", "The bytecode evaluator expected and explicit return operation", NULL, c);
	return NULL;
}
