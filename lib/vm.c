/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <string.h>
#include <stdlib.h>

NORETURN void throwStackUnderflowError(lClosure *c, const char *opcode){
	char buf[128];
	snprintf(buf, sizeof(buf), "Stack underflow at during lop%s", opcode);
	buf[sizeof(buf)-1] = 0;
	lExceptionThrowValClo("stack-underflow", buf, NULL, c);
}

/* Read an encoded signed 16-bit offset at ip */
i64 lBytecodeGetOffset16(const lBytecodeOp *ip){
	const int x = (ip[0] << 8) | ip[1];
	return (x < (1 << 15)) ? x : -((1<<16) - x);
}

/* Build a list of length len in stack starting at sp */
static lVal *lStackBuildList(lVal **stack, int sp, int len){
	if(unlikely(len == 0)){return NULL;}
	const int nsp = sp - len;
	lVal *t = stack[nsp] = lCons(stack[nsp], NULL);
	for(int i = len-1; i > 0; i--){
		t->vList.cdr = lCons(stack[sp - i], NULL);
		t = t->vList.cdr;
	}
	return stack[nsp];
}

static lVal *lDyadicFun(lBytecodeOp op, lClosure *c, lVal *a, lVal *b){
	switch(op){
	case lopLessPred:
		return lValBool((lValGreater(a, b) < 0) && !lValEqual(a, b));
	case lopLessEqPred:
		return lValBool(lValEqual(a, b) || (lValGreater(a, b) < 0));
	case lopEqualPred:
		return lValBool(lValEqual(a, b));
	case lopGreaterEqPred:
		return lValBool(lValEqual(a,b) || (lValGreater(a,b) > 0));
	case lopGreaterPred:
		return lValBool((lValGreater(a, b) > 0) && !lValEqual(a, b));
	case lopCons:
		return lCons(a,b);
	case lopIntAdd:
		return lValInt((a ? a->vInt : 0) + (b ? b->vInt : 0));
	case lopAdd:
		return lAdd(c,a,b);
	case lopSub:
		return lSub(c,a,b);
	case lopMul:
		return lMul(c,a,b);
	case lopDiv:
		return lDiv(c,a,b);
	case lopRem:
		return lRem(c,a,b);
	default:
		return NULL;
	}
}

static void lBytecodeEnsureSufficientStack(lThread *ctx){
	const int closureSizeLeft = (ctx->closureStackSize - ctx->csp) - 1;
	if(unlikely(closureSizeLeft < ctx->text->closureStackUsage)){
		ctx->closureStackSize += (((ctx->text->closureStackUsage >> 8) | 1) << 8);
		lClosure **t = realloc(ctx->closureStack, ctx->closureStackSize * sizeof(lClosure *));
		if(t == NULL){ exit(56); }
		ctx->closureStack = t;
	}

	const int valueSizeLeft = ctx->valueStackSize - ctx->sp;
	if(unlikely(valueSizeLeft < ctx->text->valueStackUsage)){
		ctx->valueStackSize += (((ctx->text->valueStackUsage >> 8) | 1) << 8);
		lVal **t = realloc(ctx->valueStack, ctx->valueStackSize * sizeof(lVal *));
		if(t == NULL){ exit(57); }
		ctx->valueStack = t;
	}
}

/* Evaluate ops within callingClosure after pushing args on the stack */
lVal *lBytecodeEval(lClosure *callingClosure, lBytecodeArray *text){
	jmp_buf oldExceptionTarget;
	lBytecodeOp *ip;
	lBytecodeArray * volatile ops = text;
	lClosure * volatile c = callingClosure;
	lThread ctx;

	if(++exceptionTargetDepth > RECURSION_DEPTH_MAX){
		exceptionTargetDepth--;
		lExceptionThrowValClo("too-deep", "Recursing too deep", NULL, NULL);
		return NULL;
	}

	ctx.closureStackSize = text->closureStackUsage;
	ctx.valueStackSize   = text->valueStackUsage;
	ctx.closureStack     = malloc(ctx.closureStackSize * sizeof(lClosure *));
	ctx.valueStack       = malloc(ctx.valueStackSize * sizeof(lVal *));
	ctx.csp              = 0;
	ctx.sp               = 0;
	ctx.closureStack[0]  = c;
	ctx.text             = text;

	int exceptionCount = 0;
	const int RSP = lRootsGet();
	lRootsClosurePush(callingClosure);
	lRootsThreadPush(&ctx);

	memcpy(oldExceptionTarget,exceptionTarget,sizeof(jmp_buf));
	const int setjmpRet = setjmp(exceptionTarget);
	if(unlikely(setjmpRet)){
		while((ctx.csp > 0) && (c->type != closureTry)){
			ctx.csp--;
			c = ctx.closureStack[ctx.csp];
		}
		if((ctx.csp > 0) && (++exceptionCount < 1000)){
			lVal *handler = c->exceptionHandler;

			c = ctx.closureStack[--ctx.csp];
			ops = c->text;
			ctx.text = ops;
			ip = c->ip;
			ctx.sp = c->sp;
			ctx.valueStack[ctx.sp++] = lApply(c, lCons(exceptionValue, NULL), handler);
		} else {
			exceptionTargetDepth--;
			memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
			free(ctx.closureStack);
			free(ctx.valueStack);
			lRootsRet(RSP);
			lExceptionThrowRaw(exceptionValue);
			return NULL;
		}
	} else {
		ip = ops->data;
		lGarbageCollectIfNecessary();
	}

	while(true){
		#ifdef VM_RUNTIME_CHECKS
		if(unlikely(ctx.csp >= ctx.closureStackSize-1)){
			ctx.closureStackSize *= 2;
			lClosure **newStack = realloc(ctx.closureStack, ctx.closureStackSize * sizeof(lClosure*));
			if (unlikely(newStack == NULL)) {
				goto topLevelNoReturn;
			}
			ctx.closureStack = newStack;
		}
		if(unlikely(ctx.sp >= ctx.valueStackSize-1)){
			ctx.valueStackSize *= 2;
			lVal **newStack = realloc(ctx.valueStack, ctx.valueStackSize * sizeof(lVal*));
			if (unlikely(newStack == NULL)) {
				goto topLevelNoReturn;
			}
			ctx.valueStack = newStack;
		}
		#endif
	switch(*ip++){
	default:
		lExceptionThrowValClo("unknown-opcode", "Stumbled upon an unknown opcode", NULL, c);
		break;
	case lopNOP:
		break;
	case lopIntByte: {
		const i8 v = *ip++;
		ctx.valueStack[ctx.sp++] = lValInt(v);
		break;}
	case lopAdd:
	case lopSub:
	case lopMul:
	case lopDiv:
	case lopRem:
	case lopIntAdd:
	case lopCons:
	case lopLessEqPred:
	case lopLessPred:
	case lopEqualPred:
	case lopGreaterEqPred:
	case lopGreaterPred: {
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lDyadicFun(ip[-1], c, a, b);
		ctx.sp--;
		break; }
	case lopPushNil:
		ctx.valueStack[ctx.sp++] = NULL;
		break;
	case lopPushTrue:
		ctx.valueStack[ctx.sp++] = lValBool(true);
		break;
	case lopPushFalse:
		ctx.valueStack[ctx.sp++] = lValBool(false);
		break;
	case lopPushValExt: {
		const uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		ctx.valueStack[ctx.sp++] = ops->literals->data[v];
		break; }
	case lopPushVal: {
		const uint v = *ip++;
		ctx.valueStack[ctx.sp++] = ops->literals->data[v];
		break; }
	case lopDup:
		ctx.sp++;
		ctx.valueStack[ctx.sp-1] = ctx.valueStack[ctx.sp-2];
		break;
	case lopDrop:
		ctx.sp--;
		break;
	case lopJmp:
		lGarbageCollectIfNecessary();
		ip += lBytecodeGetOffset16(ip)-1;
		break;
	case lopJt:
		lGarbageCollectIfNecessary();
		ip +=  castToBool(ctx.valueStack[--ctx.sp]) ? lBytecodeGetOffset16(ip)-1 : 2;
		break;
	case lopJf:
		lGarbageCollectIfNecessary();
		ip += !castToBool(ctx.valueStack[--ctx.sp]) ? lBytecodeGetOffset16(ip)-1 : 2;
		break;
	case lopDefValExt: {
		uint v;
		v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		goto defValBody;
	case lopDefVal:
		v = *ip++;
		defValBody:
		lDefineClosureSym(c, ops->literals->data[v]->vSymbol, ctx.valueStack[ctx.sp-1]);
		break; }
	case lopGetValExt: {
		uint v;
		v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		goto getValBody;
	case lopGetVal:
		v = *ip++;
		getValBody:
		ctx.valueStack[ctx.sp++] = lGetClosureSym(c, ops->literals->data[v]->vSymbol);
		break; }
	case lopSetValExt: {
		uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		goto setValBody;
	case lopSetVal:
		v = *ip++;
		setValBody:
		lSetClosureSym(c, ops->literals->data[v]->vSymbol, ctx.valueStack[ctx.sp-1]);
		break; }
	case lopZeroPred: {
		lVal *a = ctx.valueStack[ctx.sp-1];
		bool p = false;
		if(a){
			if(a->type == ltInt){
				p = a->vInt == 0;
			}else if(a->type == ltFloat){
				p = a->vFloat == 0.0;
			}
		}
		ctx.valueStack[ctx.sp-1] = lValBool(p);
		break; }
	case lopCar:
		ctx.valueStack[ctx.sp-1] = lCar(ctx.valueStack[ctx.sp-1]);
		break;
	case lopCdr:
		ctx.valueStack[ctx.sp-1] = lCdr(ctx.valueStack[ctx.sp-1]);
		break;
	case lopClosurePush:
		ctx.valueStack[ctx.sp++] = lValObject(c);
		break;
	case lopLet:
		c = lClosureNew(c, closureLet);
		c->type = closureLet;
		ctx.closureStack[++ctx.csp] = c;
		break;
	case lopClosurePop:
		c = ctx.closureStack[--ctx.csp];
		break;
	case lopTry:
		c->ip   = ip + lBytecodeGetOffset16(ip)-1;
		c->sp   = ctx.sp;
		c->text = ops;

		c = lClosureNew(c, closureTry);
		c->exceptionHandler = ctx.valueStack[--ctx.sp];
		ctx.closureStack[++ctx.csp] = c;
		ip+=2;
		break;
	case lopMacroDynamic:
	case lopFnDynamic: {
		const lBytecodeOp curOp = ip[-1];
		lVal *cBody = ctx.valueStack[--ctx.sp];
		lVal *cDocs = ctx.valueStack[--ctx.sp];
		lVal *cArgs = ctx.valueStack[--ctx.sp];
		lVal *cName = ctx.valueStack[--ctx.sp];
		lVal *fun = lLambdaNew(c, cName, cArgs, cBody);
		lClosureSetMeta(fun->vClosure, cDocs);
		ctx.valueStack[ctx.sp++] = fun;
		if(unlikely(curOp == lopMacroDynamic)){ fun->type = ltMacro; }
		break; }
	case lopApplyNew:
	case lopApply: {
		int len = *ip++;
		lVal *cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp = ctx.sp - len;
		lVal *fun = ctx.valueStack[--ctx.sp];
		if(false && fun && (fun->type == ltLambda)){
			c->sp   = ctx.sp;
			c->ip   = ip;
			c->text = ops;

			ctx.closureStack[++ctx.csp] = lClosureNewFunCall(c, cargs, fun);
			c = ctx.closureStack[ctx.csp];
			ip = c->ip;
			ctx.text = ops = c->text;
			lBytecodeEnsureSufficientStack(&ctx);
		}else{
			ctx.valueStack[ctx.sp++] = lApply(c, cargs, fun);
		}
		break; }
	case lopRet:
		if(ctx.csp > 0){
			while(ctx.closureStack[ctx.csp]->type != closureCall){
				if(--ctx.csp <= 0){goto topLevelReturn;}
			}
			lVal *ret = ctx.valueStack[ctx.sp-1];
			c   = ctx.closureStack[--ctx.csp];
			ip  = c->ip;
			ops = c->text;
			ctx.text = ops;
			ctx.sp = c->sp;
			ctx.valueStack[ctx.sp++] = ret;
			break;
		}
		topLevelReturn:
		memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
		exceptionTargetDepth--;
		lVal *ret = ctx.valueStack[ctx.sp-1];
		free(ctx.closureStack);
		free(ctx.valueStack);
		lRootsRet(RSP);
		return ret;
	}}
}
