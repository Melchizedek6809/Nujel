/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "eval.h"

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
static int pushList(lVal **stack, int sp, lVal *args){
	if(!args){return sp;}
	sp = pushList(stack, sp, lCdr(args));
	stack[sp] = lCar(args);
	return sp + 1;
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
static lBytecodeOp *lBytecodeReadOPVal(lBytecodeOp *ip, lVal **ret){
	int i = *ip++;
	i = (i << 8) | *ip++;
	i = (i << 8) | *ip++;
	*ret = lIndexVal(i);
	return ip;
}

/* Read a symbol referenced at IP and store it in RET, retuns the new IP */
static lBytecodeOp *lBytecodeReadOPSym(lBytecodeOp *ip, lSymbol **ret){
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

static void lBytecodeLinkApply(lClosure *clo, lBytecodeArray *v, lBytecodeOp *c){
	if(&c[4] >= v->dataEnd){return;}
	lVal *raw = lIndexVal((c[2] << 16) | (c[3] << 8) | c[4]);
	if((!raw) || (raw->type != ltSymbol)){return;}
	lVal *n = lGetClosureSym(clo, raw->vSymbol);
	if(n == raw){return;}
	int i = lValIndex(n);
	c[2] = (i >> 16) & 0xFF;
	c[3] = (i >>  8) & 0xFF;
	c[4] = (i      ) & 0xFF;
}

static void lBytecodeLinkPush(lClosure *clo, lBytecodeArray *v, lBytecodeOp *c){
	if(&c[3] >= v->dataEnd){return;}
	lVal *raw = lIndexVal((c[1] << 16) | (c[2] << 8) | c[3]);
	if(!raw || (raw->type != ltSymbol)){return;}
	lVal *n = lGetClosureSym(clo, raw->vSymbol);
	if(n == raw){return;}
	int i = lValIndex(n);
	c[1] = (i >> 16) & 0xFF;
	c[2] = (i >>  8) & 0xFF;
	c[3] = (i      ) & 0xFF;
}

static void lBytecodeTrace(const lThread *ctx, const lBytecodeOp *ip, const lBytecodeArray *ops){
	pf("[OP: %u]\n [IP: %x, CSP:%i SP:%i]\n", (ip - ops->data), (i64)*ip, (i64)ctx->csp, (i64)ctx->sp);
	for(int i=ctx->csp;i>=0;i--){
		pf("C %u: %i\n", i, (i64)(ctx->closureStack[i] ? ctx->closureStack[i]->type : 0));
	}
	for(int i=ctx->sp-1;i>=0;i--){
		pf("V %u: %V\n", i, ctx->valueStack[i]);
	}
	pf("\n");
	if(ctx->csp < 0){
		epf("CSP Error!");
		exit(1);
	}
	if(ctx->sp < 0){
		epf("SP Error!");
		exit(1);
	}
}

/* Evaluate ops within callingClosure after pushing args on the stack */
lVal *lBytecodeEval(lClosure *callingClosure, lVal *args, lBytecodeArray *ops, bool trace){
	jmp_buf oldExceptionTarget;
	lBytecodeOp *ip;
	lClosure * volatile c = callingClosure;
	lThread ctx;
	ctx.closureStackSize = 4;
	ctx.valueStackSize = 8;
	ctx.closureStack = calloc(ctx.closureStackSize, sizeof(lClosure *));
	ctx.valueStack = calloc(ctx.valueStackSize, sizeof(lVal *));
	ctx.csp = 0;
	ctx.sp = 0;
	ctx.closureStack[ctx.csp] = c;

	int exceptionCount = 0;
	lRootsThreadPush(&ctx);

	memcpy(oldExceptionTarget,exceptionTarget,sizeof(jmp_buf));
	exceptionTargetDepth++;
	const int setjmpRet = setjmp(exceptionTarget);
	if(setjmpRet){
		while((ctx.csp > 0) && (c->type != closureTry) && (c->type != closureTry)){
			ctx.csp--;
			c = ctx.closureStack[ctx.csp];
		}
		if((ctx.csp > 0) && (++exceptionCount < 1000) && (c->type == closureTry)){
			ip = c->ip;
			ctx.sp = c->sp;
			lVal *handler = RVP(c->exceptionHandler);
			c = ctx.closureStack[--ctx.csp];
			ctx.valueStack[ctx.sp++] = lApply(c, lCons(exceptionValue, NULL), handler);
		}else{
			memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
			free(ctx.closureStack);
			free(ctx.valueStack);
			lRootsRet(callingClosure->rsp);
			lExceptionThrowRaw(exceptionValue);
			return NULL;
		}
	}else{
		ip = ops->data;
		(void)args;
		ctx.sp = pushList(ctx.valueStack, 0, args);
	}

	while((ip >= ops->data) && (ip < ops->dataEnd)){
		if(ctx.csp == ctx.closureStackSize-1){
			ctx.closureStackSize *= 2;
			ctx.closureStack = realloc(ctx.closureStack,ctx.closureStackSize * sizeof(lClosure *));
		}
		if(ctx.sp == ctx.valueStackSize-1){
			ctx.valueStackSize *= 2;
			ctx.valueStack = realloc(ctx.valueStack,ctx.valueStackSize * sizeof(lVal *));
		}
		if(trace){lBytecodeTrace(&ctx, ip, ops);}
	switch(*ip){
	default:
		lExceptionThrowValClo("unknown-opcode", "Stubmbled upon an unknown opcode", NULL, c);
		break;
	case lopNOP:
		ip++;
		break;
	case lopIntByte: {
		const i8 v = ip[1];
		ctx.valueStack[ctx.sp++] = lValInt(v);
		ip+=2;
		break;}
	case lopIntAdd: {
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopIntAdd", NULL, c);}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		if((a == NULL) || (b == NULL)){lExceptionThrowValClo("type-error", "Can't add #nil", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lValInt(a->vInt + b->vInt);
		ctx.sp--;
		ip++;
		break; }
	case lopLessPred: {
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopLessPred", NULL, c);}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool((lValGreater(a, b) < 0) && !lValEqual(a, b));
		ctx.sp--;
		ip++;
		break; }
	case lopLessEqPred: {
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopLessEqPred", NULL, c);}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(a, b) || (lValGreater(a, b) < 0));
		ctx.sp--;
		ip++;
		break; }
	case lopEqualPred: {
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopEqualPred", NULL, c);}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(a, b));
		ctx.sp--;
		ip++;
		break; }
	case lopGreaterEqPred: {
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopGreaterEqPred", NULL, c);}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(a,b) || lValGreater(a,b) > 0);
		ctx.sp--;
		ip++;
		break; }
	case lopGreaterPred: {
		if(ctx.sp < 2){lExceptionThrowValClo("stack-underflow", "Underflow during lopGreaterPred", NULL, c);}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool((lValGreater(a, b) > 0) && !lValEqual(a, b));
		ctx.sp--;
		ip++;
		break; }
	case lopPushNil:
		ctx.valueStack[ctx.sp++] = NULL;
		ip++;
		break;
	case lopPushLVal: {
		lVal *val = lIndexVal((ip[1] << 16) | (ip[2] << 8) | ip[3]);
		if(val && (val->type == ltSymbol)){
			lBytecodeLinkPush(c, ops, ip);
			val = lIndexVal((ip[1] << 16) | (ip[2] << 8) | ip[3]);
		}
		ctx.valueStack[ctx.sp++] = val;
		ip += 4;
		break; }
	case lopPushSymbol: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip+1, &sym);
		ctx.valueStack[ctx.sp++] = lValSymS(sym);
		break;}
	case lopDup:
		if(ctx.sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed during lopDup", NULL, c);}
		ctx.valueStack[ctx.sp] = ctx.valueStack[ctx.sp-1];
		ctx.sp++;
		ip++;
		break;
	case lopDrop:
		if(ctx.sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed during lopDrop", NULL, c);}
		ctx.sp--;
		ip++;
		break;
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
	case lopMacroAst:
	case lopFn: {
		const lBytecodeOp curOp = *ip;
		lVal *cName, *cArgs, *cDocs, *cBody;
		ip = lBytecodeReadOPVal(ip+1, &cName);
		ip = lBytecodeReadOPVal(ip,   &cArgs);
		ip = lBytecodeReadOPVal(ip,   &cDocs);
		ip = lBytecodeReadOPVal(ip,   &cBody);
		ctx.valueStack[ctx.sp] = lLambdaNew(c, cName, cArgs, cBody);
		lClosureSetMeta(ctx.valueStack[ctx.sp]->vClosure, cDocs);
		if(curOp == lopMacroAst){ctx.valueStack[ctx.sp]->type = ltMacro;}
		ctx.sp++;
		break;}
	case lopRootsSave:
		c->rsp = lRootsGet();
		ip++;
		break;
	case lopRootsRestore:
		lRootsRet(c->rsp);
		ip++;
		break;
	case lopCar:
		if(ctx.sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed during car", NULL, c);}
		ctx.valueStack[ctx.sp-1] = lCar(ctx.valueStack[ctx.sp-1]);
		ip++;
		break;
	case lopCdr:
		if(ctx.sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed during cdr", NULL, c);}
		ctx.valueStack[ctx.sp-1] = lCdr(ctx.valueStack[ctx.sp-1]);
		ip++;
		break;
	case lopCons:
		if(ctx.sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed during cons", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lCons(ctx.valueStack[ctx.sp-2],ctx.valueStack[ctx.sp-1]);
		ctx.sp--;
		ip++;
		break;
	case lopClosurePush:
		ctx.valueStack[ctx.sp++] = lValObject(c);
		ip++;
		break;
	case lopLet:
		c = lClosureNew(c, closureLet);
		c->type = closureLet;
		ctx.closureStack[++ctx.csp] = c;
		ip++;
		break;
	case lopClosurePop: {
		lClosure *nclo = ctx.closureStack[--ctx.csp];
		c = nclo;
		lRootsRet(c->rsp);
		if(ctx.csp < 0){lExceptionThrowValClo("stack-underflow", "Underflowed during lopClosurePop", NULL, c);}
		ip++;
		break; }
	case lopTry:
		if(ctx.sp < 1){lExceptionThrowValClo("stack-underflow", "Underflowed during try", NULL, c);}
		c = lClosureNew(c, closureTry);
		c->exceptionHandler = ctx.valueStack[--ctx.sp];
		c->ip = ip + lBytecodeGetOffset16(ip+1);
		c->sp = ctx.sp;
		ctx.closureStack[++ctx.csp] = c;
		ip+=3;
		break;
	case lopApplyDynamic:
	case lopApply: {
		const lBytecodeOp curOp = *ip;
		const int len = ip[1];
		lVal *cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp = ctx.sp - len + 1;
		ctx.valueStack[ctx.sp-1] = cargs;
		lVal *fun;
		if(curOp == lopApply){
			fun = lIndexVal((ip[2] << 16) | (ip[3] << 8) | ip[4]);
			if(fun && (fun->type == ltSymbol)){
				lBytecodeLinkApply(c, ops, ip);
				fun = lIndexVal((ip[2] << 16) | (ip[3] << 8) | ip[4]);
			}
			ip += 5;
		}else{
			fun = ctx.valueStack[ctx.sp-2];
			ip+=2;
			ctx.sp--;
		}
		ctx.valueStack[ctx.sp-1] = lApply(c, cargs, fun);
		break; }
	case lopApplyNew: {
		const int len = ip[1];
		lVal *cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp = ctx.sp - len + 1;
		ctx.valueStack[ctx.sp-1] = cargs;
		lVal *fun = lIndexVal((ip[2] << 16) | (ip[3] << 8) | ip[4]);
		if(fun && (fun->type == ltSymbol)){
			lBytecodeLinkApply(c, ops, ip);
			fun = lIndexVal((ip[2] << 16) | (ip[3] << 8) | ip[4]);
		}
		ip += 5;
		ctx.valueStack[ctx.sp-1] = lApply(c, cargs, fun);
		break; }
	case lopRet:
		if(ctx.sp < 1){
			lExceptionThrowValClo("stack-underflow", "Underflow during lopRet", NULL, c);
			return NULL;
		}
		memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
		exceptionTargetDepth--;
		lRootsRet(callingClosure->rsp);
		lVal *ret = RVP(ctx.valueStack[ctx.sp-1]);
		free(ctx.closureStack);
		free(ctx.valueStack);
		return ret;
	}}
	memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
	exceptionTargetDepth--;
	lRootsRet(callingClosure->rsp);
	free(ctx.closureStack);
	free(ctx.valueStack);
	lExceptionThrowValClo("no-return", "The bytecode evaluator needs an explicit return", NULL, c);
	return NULL;
}
