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
#include <stdlib.h>

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
	lClosure * volatile c = callingClosure;
	lContext ctx;
	ctx.closureStackSize = 16;
	ctx.valueStackSize = 32;
	ctx.closureStack = malloc(sizeof(lClosure *) * ctx.closureStackSize);
	ctx.valueStack = malloc(sizeof(lVal *) * ctx.valueStackSize);
	ctx.closureStack[0] = c;
	ctx.csp = 1;
	ctx.sp = 0;

	int exceptionCount = 0;
	c->type = closureLet;
	lRootsContextPush(&ctx);

	memcpy(oldExceptionTarget,exceptionTarget,sizeof(jmp_buf));
	exceptionTargetDepth++;
	const int setjmpRet = setjmp(exceptionTarget);
	if(setjmpRet){
		while((ctx.csp >= 0) && (c->type != closureTry)){
			c = ctx.closureStack[--ctx.csp];
		}
		if((ctx.csp >= 0) && (++exceptionCount < 100000) && (c->type == closureTry)){
			ip = c->ip;
			ctx.sp = c->sp;
			ctx.valueStack[ctx.sp++] = exceptionValue;
			c = ctx.closureStack[--ctx.csp];
		}else{
			memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
			lExceptionThrowRaw(exceptionValue);
			return NULL;
		}
	}else{
		ip = ops->data;
		(void)args;
		ctx.sp = pushList(ctx.valueStack, 0, args);
	}

	while((ip >= ops->data) && (ip < ops->dataEnd)){
		if(ctx.csp == ctx.closureStackSize){
			ctx.closureStackSize += MIN(ctx.closureStackSize, 4096);
			ctx.closureStack = realloc(ctx.closureStack,ctx.closureStackSize * sizeof(lClosure *));
		}
		if(ctx.sp == ctx.valueStackSize){
			ctx.valueStackSize += MIN(ctx.valueStackSize, 8192);
			ctx.valueStack = realloc(ctx.valueStack,ctx.valueStackSize * sizeof(lVal *));
		}
	switch(*ip){
	default:
		lExceptionThrowValClo("unknown-opcode", "Stubmbled upon an unknown opcode", NULL, c);
		break;
	case lopNOP:
		ip++;
		break;
	case lopIntByte: {
		const i8 v = *++ip;
		ctx.valueStack[ctx.sp++] = lValInt(v);
		ip++;
		break;}
	case lopIntAdd:
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopIntAdd", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lValInt(castToInt(ctx.valueStack[ctx.sp-2],0) + castToInt(ctx.valueStack[ctx.sp-1],0));
		ctx.sp--;
		ip++;
		break;
	case lopLessPred:
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopLessPred", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lValBool(lValGreater(ctx.valueStack[ctx.sp-2], ctx.valueStack[ctx.sp-1]) < 0);
		ctx.sp--;
		ip++;
		break;
	case lopLessEqPred:
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopLessEqPred", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(ctx.valueStack[ctx.sp-2],ctx.valueStack[ctx.sp-1])
			|| (lValGreater(ctx.valueStack[ctx.sp-2],ctx.valueStack[ctx.sp-1]) < 0));
		ctx.sp--;
		ip++;
		break;
	case lopEqualPred:
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopEqualPred", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(ctx.valueStack[ctx.sp-2],ctx.valueStack[ctx.sp-1]));
		ctx.sp--;
		ip++;
		break;
	case lopGreaterEqPred:
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopGreaterEqPred", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(ctx.valueStack[ctx.sp-2],ctx.valueStack[ctx.sp-1])
			|| lValGreater(ctx.valueStack[ctx.sp-2],ctx.valueStack[ctx.sp-1]) > 0);
		ctx.sp--;
		ip++;
		break;
	case lopGreaterPred:
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopGreaterPred", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lValBool(lValGreater(ctx.valueStack[ctx.sp-2],ctx.valueStack[ctx.sp-1]) > 0);
		ctx.sp--;
		ip++;
		break;
	case lopDebugPrintStack:
		pf("Bytecode Debug stack:\n");
		printStack(ctx.valueStack, ctx.sp, ctx.closureStack, ctx.csp);
		ip++;
		break;
	case lopPushNil:
		ctx.valueStack[ctx.sp++] = NULL;
		ip++;
		break;
	case lopPushLVal:
		ip = lBytecodeReadOPVal(ip+1, &ctx.valueStack[ctx.sp++]);
		break;
	case lopPushSymbol: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		ctx.valueStack[ctx.sp++] = lValSymS(sym);
		break;}
	case lopMakeList: {
		const int len = *++ip;
		lVal *ret = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp -= len;
		ctx.valueStack[ctx.sp++] = ret;
		ip++;
		break;}
	case lopEval:
		ctx.valueStack[ctx.sp-1] = lEval(c,ctx.valueStack[ctx.sp-1]);
		ip++;
		break;
	case lopApply: {
		const int len = ip[1];
		lVal *cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp = ctx.sp - len + 1;
		lVal *fun = lIndexVal((ip[2] << 16) | (ip[3] << 8) | ip[4]);
		ip += 5;
		ctx.valueStack[ctx.sp-1] = cargs;
		ctx.valueStack[ctx.sp-1] = lApply(c, cargs, fun, fun);
		break; }
	case lopApplyDynamic: {
		const int len = ip[1];
		lVal *cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp = ctx.sp - len + 1;
		ctx.valueStack[ctx.sp-1] = cargs;
		lVal *fun = ctx.valueStack[ctx.sp-2];
		ip+=2;
		ctx.valueStack[ctx.sp-2] = lApply(c, cargs, fun, fun);
		ctx.sp--;
		break; }
	case lopDup:
		if(ctx.sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed during lopDup", NULL, c);}
		ctx.valueStack[ctx.sp] = ctx.valueStack[ctx.sp-1];
		ctx.sp++;
		ip++;
		break;
	case lopDrop:
		if(--ctx.sp < 0){lExceptionThrowValClo("stack-underflow", "Underflowed during lopDrop", NULL, c);}
		ip++;
		break;
	case lopSwap: {
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflowed during lopDup", NULL, c);}
		lVal *t = ctx.valueStack[ctx.sp-2];
		ctx.valueStack[ctx.sp-2] = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-1] = t;
		ip++;
		break; }
	case lopJmp:
		ip += lBytecodeGetOffset16(ip+1);
		break;
	case lopJt:
		ip +=  castToBool(ctx.valueStack[--ctx.sp]) ? lBytecodeGetOffset16(ip+1) : 3;
		break;
	case lopJf:
		ip += !castToBool(ctx.valueStack[--ctx.sp]) ? lBytecodeGetOffset16(ip+1) : 3;
		break;
	case lopDef: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		lDefineClosureSym(c, sym, ctx.valueStack[ctx.sp - 1]);
		break; }
	case lopSet: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		lSetClosureSym(c, sym, ctx.valueStack[ctx.sp - 1]);
		break; }
	case lopGet: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		ctx.valueStack[ctx.sp++] = lGetClosureSym(c, sym);
		break; }
	case lopLambda: {
		lVal *cName, *cArgs, *cDocs, *cBody;
		ip = lBytecodeReadOPVal(ip+1, &cName);
		ip = lBytecodeReadOPVal(ip, &cArgs);
		ip = lBytecodeReadOPVal(ip, &cDocs);
		ip = lBytecodeReadOPVal(ip, &cBody);
		ctx.valueStack[ctx.sp++] = lLambdaNew(c, cName, cArgs, cDocs, cBody);
		break;}
	case lopFn: {
		lVal *cName, *cArgs, *cDocs, *cBody;
		ip = lBytecodeReadOPVal(ip+1, &cName);
		ip = lBytecodeReadOPVal(ip, &cArgs);
		ip = lBytecodeReadOPVal(ip, &cDocs);
		ip = lBytecodeReadOPVal(ip, &cBody);
		ctx.valueStack[ctx.sp++] = lLambdaBytecodeNew(c, cName, cArgs, cDocs, cBody);
		break;}
	case lopMacro: {
		lVal *cName, *cArgs, *cDocs, *cBody;
		ip = lBytecodeReadOPVal(ip+1, &cName);
		ip = lBytecodeReadOPVal(ip, &cArgs);
		ip = lBytecodeReadOPVal(ip, &cDocs);
		ip = lBytecodeReadOPVal(ip, &cBody);
		ctx.valueStack[ctx.sp] = lLambdaNew(c, cName, cArgs, cDocs, cBody);
		if(ctx.valueStack[ctx.sp]){
			ctx.valueStack[ctx.sp]->type = ltMacro;
		}
		ctx.sp++;
		break; }
	case lopMacroAst: {
		lVal *cName, *cArgs, *cDocs, *cBody;
		ip = lBytecodeReadOPVal(ip+1, &cName);
		ip = lBytecodeReadOPVal(ip, &cArgs);
		ip = lBytecodeReadOPVal(ip, &cDocs);
		ip = lBytecodeReadOPVal(ip, &cBody);
		ctx.valueStack[ctx.sp] = lLambdaBytecodeNew(c, cName, cArgs, cDocs, cBody);
		if(ctx.valueStack[ctx.sp]){
			ctx.valueStack[ctx.sp]->type = ltMacro;
		}
		ctx.sp++;
		break; }
	case lopRootsSave:
		c->rsp = lRootsGet();
		ip++;
		break;
	case lopRootsRestore:
		lRootsRet(c->rsp);
		ip++;
		break;
	case lopClosurePush:
		ip++;
		ctx.valueStack[ctx.sp++] = lValObject(c);
		break;
	case lopClosureEnter: {
		ip++;
		lVal *cObj = ctx.valueStack[--ctx.sp];
		if((cObj->type != ltLambda) && (cObj->type != ltObject)){
			lExceptionThrowValClo("invalid-closure", "Error while trying to enter a closure", cObj, c);
		}
		ctx.closureStack[ctx.csp++] = c;
		c = cObj->vClosure;
		ctx.closureStack[ctx.csp] = c;
		break; }
	case lopLet:
		ip++;
		ctx.closureStack[ctx.csp++] = c;
		c = lClosureNew(c);
		c->type = closureLet;
		ctx.closureStack[ctx.csp] = c;
		break;
	case lopClosurePop: {
		ip++;
		lClosure *nclo = ctx.closureStack[--ctx.csp];
		c = nclo;
		lRootsRet(c->rsp);
		if(ctx.csp == 0){
			if(ctx.sp < 1){
				lExceptionThrowValClo("stack-underflow", "Underflowed during lopClosurePop", NULL, c);
			}
			return NULL;
		}
		break; }
	case lopCall:
		c->ip = ip+3;
		c->sp = ctx.sp;
		ctx.closureStack[ctx.csp++] = c;
		c = lClosureNew(c);
		c->type = closureCall;
		ctx.closureStack[ctx.csp] = c;
		ip += lBytecodeGetOffset16(ip+1);
		break;
	case lopTry:
		ctx.closureStack[ctx.csp++] = c;
		c = lClosureNew(c);
		c->type = closureTry;
		c->ip = ip + lBytecodeGetOffset16(ip+1);
		c->sp = ctx.sp;
		ctx.closureStack[ctx.csp] = c;
		ip+=3;
		break;
	case lopRet:
		while(ctx.csp > 0){
			lVal *rv = ctx.valueStack[ctx.sp-1];
			lClosure *nclo = ctx.closureStack[--ctx.csp];
			if(ctx.csp == 0){break;}
			if((nclo->type == closureTry)
			|| (nclo->type == closureLet)){
				continue;
			}
			ip = nclo->ip;
			ctx.sp = nclo->sp;
			lRootsRet(nclo->rsp);
			ctx.valueStack[ctx.sp++] = rv;
			break;
		}
		if(ctx.csp == 0){
			if(ctx.sp < 1){
				lExceptionThrowValClo("stack-underflow", "Underflow during lopRet", NULL, c);
				return NULL;
			}
			memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
			exceptionTargetDepth--;
			lRootsRet(ctx.closureStack[0]->rsp);
			return ctx.valueStack[--ctx.sp];
		}
		break;
	case lopThrow:
		ip++;
		while(ctx.csp > 0){
			lVal *rv = ctx.valueStack[ctx.sp-1];
			lClosure *nclo = ctx.closureStack[--ctx.csp];
			if(!nclo || ctx.csp == 0){break;}
			if(nclo->type != closureTry){continue;}
			ip = nclo->ip;
			ctx.sp = nclo->sp;
			lRootsRet(nclo->rsp);
			ctx.valueStack[ctx.sp++] = rv;
			break;
		}
		if(ctx.csp == 0){
			if(ctx.sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed during lopThrow", NULL, c);}
			lExceptionThrowRaw(ctx.valueStack[--ctx.sp]);
			return NULL;
		}
		break;
	}}
	lExceptionThrowValClo("no-return", "The bytecode evaluator needs an explicit return", NULL, c);
	return NULL;
}

/* Return the overall length of opcode op */
static int lBytecodeOpLength(lBytecodeOp op){
	switch(op){
	default:
		fprintf(stderr,"Unknown bytecodeOp length: %x\n",op);
		exit(3);
	case lopNOP:
	case lopRet:
	case lopIntAdd:
	case lopDebugPrintStack:
	case lopEval:
	case lopDup:
	case lopDrop:
	case lopClosurePush:
	case lopClosureEnter:
	case lopLet:
	case lopClosurePop:
	case lopThrow:
	case lopRootsSave:
	case lopRootsRestore:
	case lopLessPred:
	case lopLessEqPred:
	case lopEqualPred:
	case lopGreaterEqPred:
	case lopGreaterPred:
	case lopPushNil:
	case lopSwap:
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
	case lopMacroAst:
	case lopFn:
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
		case lopMacroAst:
		case lopFn:
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
