/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "eval.h"

#include "bytecode.h"
#include "tracing.h"
#include "../operations.h"
#include "../printer.h"
#include "../type/closure.h"
#include "../type/val.h"

#include <string.h>
#include <stdlib.h>


NORETURN void throwStackUnderflowError(lClosure *c, const char *opcode){
	char buf[128];
	spf(buf, &buf[sizeof(buf)], "Stack underflow at during lop%s", opcode);
	lExceptionThrowValClo("stack-underflow", buf, NULL, c);
}

/* Build a list of length len in stack starting at sp */
static lVal *lStackBuildList(lVal **stack, int sp, int len){
	if(len == 0){return lCons(NULL, NULL);}
	const int nsp = sp - len;
	lVal *t = stack[nsp] = lRootsValPush(lCons(stack[nsp], NULL));
	for(int i = len-1; i > 0; i--){
		t->vList.cdr = lCons(stack[sp - i], NULL);
		t = t->vList.cdr;
	}
	return stack[nsp];
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

static void lBytecodeArrayLink(lClosure *c, lBytecodeArray *text){
	if(text->literals){
		for(int i=0;i<text->literals->length;i++){
			if(text->literals->data[i] == NULL){continue;}
			if(text->literals->data[i]->type != ltSymbol){continue;}
			lVal *new = lGetClosureSym(c, text->literals->data[i]->vSymbol);
			text->literals->data[i] = new;
		}
	}
	text->flags |= BYTECODE_ARRAY_LINKED;
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

/* Evaluate ops within callingClosure after pushing args on the stack */
lVal *lBytecodeEval(lClosure *callingClosure, lBytecodeArray *text, bool trace){
	jmp_buf oldExceptionTarget;
	lBytecodeOp *ip;
	lBytecodeArray * volatile ops = text;
	lClosure * volatile c = callingClosure;
	lThread ctx;
	ctx.closureStackSize = 4;
	ctx.valueStackSize   = 8;
	ctx.closureStack     = calloc(ctx.closureStackSize, sizeof(lClosure *));
	ctx.valueStack       = calloc(ctx.valueStackSize,   sizeof(lVal *));
	ctx.csp              = 0;
	ctx.sp               = 0;
	ctx.closureStack[ctx.csp] = c;
	ctx.text             = text;

	if(!(text->flags & BYTECODE_ARRAY_LINKED)){
		lBytecodeArrayLink(c,text);
	}

	int exceptionCount = 0;
	const int RSP = lRootsGet();
	lRootsClosurePush(callingClosure);
	lRootsThreadPush(&ctx);

	memcpy(oldExceptionTarget,exceptionTarget,sizeof(jmp_buf));
	exceptionTargetDepth++;
	const int setjmpRet = setjmp(exceptionTarget);
	if(setjmpRet){
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
			memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
			free(ctx.closureStack);
			free(ctx.valueStack);
			lExceptionThrowRaw(exceptionValue);
			lRootsRet(RSP);
			return NULL;
		}
	} else {
		ip = ops->data;
	}

	while((ip >= ops->data) && (ip < ops->dataEnd)){
		lGarbageCollectIfNecessary();
		if(ctx.csp >= ctx.closureStackSize-1){
			ctx.closureStackSize *= 2;
			ctx.closureStack = realloc(ctx.closureStack,ctx.closureStackSize * sizeof(lClosure *));
		}
		if(ctx.sp >= ctx.valueStackSize-1){
			ctx.valueStackSize *= 2;
			ctx.valueStack = realloc(ctx.valueStack,ctx.valueStackSize * sizeof(lVal *));
		}
		if(trace){lBytecodeTrace(&ctx, ip, ops);}
	switch(*ip++){
	default:
		lExceptionThrowValClo("unknown-opcode", "Stubmbled upon an unknown opcode", NULL, c);
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
		if(ctx.sp < 2){throwStackUnderflowError(c, lBytecodeGetOpcodeName(ip[-1]));}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lDyadicFun(ip[-1], c, a, b);
		ctx.sp--;
		break; }
	case lopPushNil:
		ctx.valueStack[ctx.sp++] = NULL;
		break;
	case lopPushLVal: {
		lVal *val = lIndexVal((ip[0] << 16) | (ip[1] << 8) | ip[2]);
		if(val && (val->type == ltSymbol)){
			lBytecodeLinkPush(c, ops, ip-1);
			val = lIndexVal((ip[0] << 16) | (ip[1] << 8) | ip[2]);
		}
		ctx.valueStack[ctx.sp++] = val;
		ip += 3;
		break; }
	case lopPushVal: {
		const u8 v = *ip++;
		if(v >= ops->literals->length){throwStackUnderflowError(c, "PushVal");}
		ctx.valueStack[ctx.sp++] = ops->literals->data[v];
		break; }
	case lopPushSymbol: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip, &sym);
		ctx.valueStack[ctx.sp++] = lValSymS(sym);
		break;}
	case lopDup:
		if(ctx.sp < 1){throwStackUnderflowError(c, "Dup");}
		ctx.sp++;
		ctx.valueStack[ctx.sp-1] = ctx.valueStack[ctx.sp-2];
		break;
	case lopDrop:
		if(ctx.sp < 1){throwStackUnderflowError(c, "Drop");}
		ctx.sp--;
		break;
	case lopJmp:
		ip += lBytecodeGetOffset16(ip)-1;
		break;
	case lopJt:
		ip +=  castToBool(ctx.valueStack[--ctx.sp]) ? lBytecodeGetOffset16(ip)-1 : 2;
		break;
	case lopJf:
		ip += !castToBool(ctx.valueStack[--ctx.sp]) ? lBytecodeGetOffset16(ip)-1 : 2;
		break;
	case lopDef: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip, &sym);
		lDefineClosureSym(c, sym, ctx.valueStack[ctx.sp - 1]);
		break; }
	case lopSet: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip, &sym);
		lSetClosureSym(c, sym, ctx.valueStack[ctx.sp - 1]);
		break; }
	case lopGet: {
		lSymbol *sym;
		ip = lBytecodeReadOPSym(ip, &sym);
		ctx.valueStack[ctx.sp++] = lGetClosureSym(c, sym);
		break; }
	case lopZeroPred: {
		if(ctx.sp < 1){throwStackUnderflowError(c, "zero?");}
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
		if(ctx.sp < 1){throwStackUnderflowError(c, "Car");}
		ctx.valueStack[ctx.sp-1] = lCar(ctx.valueStack[ctx.sp-1]);
		break;
	case lopCdr:
		if(ctx.sp < 1){throwStackUnderflowError(c, "Cdr");}
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
	case lopClosurePop: {
		lClosure *nclo = ctx.closureStack[--ctx.csp];
		c = nclo;
		if(ctx.csp < 0){throwStackUnderflowError(c, "ClosurePop");}
		break; }
	case lopTry:
		if(ctx.sp < 1){throwStackUnderflowError(c, "Try");}

		c->ip   = ip + lBytecodeGetOffset16(ip)-1;
		c->sp   = ctx.sp;
		c->rsp  = lRootsGet();
		c->text = ops;

		c = lClosureNew(c, closureTry);
		c->exceptionHandler = ctx.valueStack[--ctx.sp];
		ctx.closureStack[++ctx.csp] = c;
		ip+=2;
		break;
	case lopMacroDynamic:
	case lopFnDynamic: {
		if(ctx.sp < 4){throwStackUnderflowError(c, "Fn");}
		const lBytecodeOp curOp = ip[-1];
		lVal *cBody = ctx.valueStack[--ctx.sp];
		lVal *cDocs = ctx.valueStack[--ctx.sp];
		lVal *cArgs = ctx.valueStack[--ctx.sp];
		lVal *cName = ctx.valueStack[--ctx.sp];
		if(cBody->type == ltNoAlloc){
			pf("SP: %i CSP: %i\n", (i64)ctx.sp, (i64)ctx.csp);
			pf("%V\n",cBody);
			pf("%V\n",cDocs);
			pf("%V\n",cArgs);
			pf("%V\n",cName);
			//*((u8 *)NULL)=0;
		}
		lVal *fun = lLambdaNew(c, cName, cArgs, cBody);
		lClosureSetMeta(fun->vClosure, cDocs);
		ctx.valueStack[ctx.sp++] = fun;
		if(curOp == lopMacroDynamic){ fun->type = ltMacro; }
		break;}
	case lopApply: {
		const int len = *ip++;
		if(ctx.sp < len){
			throwStackUnderflowError(c, "Apply");
		}
		const int applyRSP = lRootsGet();
		lVal *cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp = ctx.sp - len;
		lVal *fun = ctx.valueStack[--ctx.sp];
		lVal *res = lApply(c, cargs, fun);
		ctx.valueStack[ctx.sp++] = res;
		(void)applyRSP;
		//lRootsRet(applyRSP);
		break; }
	case lopRet:
		if(ctx.sp < 1){ throwStackUnderflowError(c, "Ret"); }
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
	memcpy(exceptionTarget, oldExceptionTarget, sizeof(jmp_buf));
	exceptionTargetDepth--;
	free(ctx.closureStack);
	free(ctx.valueStack);
	lExceptionThrowValClo("no-return", "The bytecode evaluator needs an explicit return", NULL, c);
	memset(&ctx, 0, sizeof(lThread));
	lRootsRet(RSP);
	return NULL;
}
