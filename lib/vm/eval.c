/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "eval.h"

#include "bytecode.h"
#include "tracing.h"
#include "../printer.h"
#include "../type/closure.h"
#include "../type/val.h"

#include <string.h>
#include <stdlib.h>

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

NORETURN void throwStackUnderflowError(lClosure *c, const char *opcode){
	char buf[128];
	spf(buf, &buf[sizeof(buf)], "Stack underflow at during lop%s", opcode);
	lExceptionThrowValClo("stack-underflow", buf, NULL, c);
}

/* Evaluate ops within callingClosure after pushing args on the stack */
lVal *lBytecodeEval(lClosure *callingClosure, lBytecodeArray *text, bool trace){
	jmp_buf oldExceptionTarget;
	lBytecodeOp *ip;
	lBytecodeArray * volatile ops = text;
	lClosure * volatile c = callingClosure;
	lThread ctx;
	ctx.magicValue = 0xbaddbeefcafebabe;
	ctx.closureStackSize = 4;
	ctx.valueStackSize   = 8;
	ctx.closureStack     = calloc(ctx.closureStackSize, sizeof(lClosure *));
	ctx.valueStack       = calloc(ctx.valueStackSize,   sizeof(lVal *));
	ctx.csp = 0;
	ctx.sp  = 0;
	ctx.closureStack[ctx.csp] = c;

	int exceptionCount = 0;
	const int RSP = lRootsGet();
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
		if(ctx.sp < 2){throwStackUnderflowError(c, "IntAdd");}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		if((a == NULL) || (b == NULL)){lExceptionThrowValClo("type-error", "Can't add #nil", NULL, c);}
		ctx.valueStack[ctx.sp-2] = lValInt(a->vInt + b->vInt);
		ctx.sp--;
		ip++;
		break; }
	case lopLessPred: {
		if(ctx.sp < 2){throwStackUnderflowError(c, "LessPred");}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool((lValGreater(a, b) < 0) && !lValEqual(a, b));
		ctx.sp--;
		ip++;
		break; }
	case lopLessEqPred: {
		if(ctx.sp < 2){throwStackUnderflowError(c, "LessEqPred");}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(a, b) || (lValGreater(a, b) < 0));
		ctx.sp--;
		ip++;
		break; }
	case lopEqualPred: {
		if(ctx.sp < 2){throwStackUnderflowError(c, "EqualPred");}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(a, b));
		ctx.sp--;
		ip++;
		break; }
	case lopGreaterEqPred: {
		if(ctx.sp < 2){throwStackUnderflowError(c, "GreaterEqPred");}
		lVal *a = ctx.valueStack[ctx.sp-2];
		lVal *b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(a,b) || lValGreater(a,b) > 0);
		ctx.sp--;
		ip++;
		break; }
	case lopGreaterPred: {
		if(ctx.sp < 2){throwStackUnderflowError(c, "GreaterPred");}
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
		if(ctx.sp < 1){throwStackUnderflowError(c, "Dup");}
		ctx.valueStack[ctx.sp] = ctx.valueStack[ctx.sp-1];
		ctx.sp++;
		ip++;
		break;
	case lopDrop:
		if(ctx.sp < 1){throwStackUnderflowError(c, "Drop");}
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
		ctx.valueStack[ctx.sp++] = lLambdaNew(c, cName, cArgs, cBody);
		lClosureSetMeta(ctx.valueStack[ctx.sp-1]->vClosure, cDocs);
		if(curOp == lopMacroAst){ctx.valueStack[ctx.sp-1]->type = ltMacro;}
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
		if(ctx.sp < 1){throwStackUnderflowError(c, "Car");}
		ctx.valueStack[ctx.sp-1] = lCar(ctx.valueStack[ctx.sp-1]);
		ip++;
		break;
	case lopCdr:
		if(ctx.sp < 1){throwStackUnderflowError(c, "Cdr");}
		ctx.valueStack[ctx.sp-1] = lCdr(ctx.valueStack[ctx.sp-1]);
		ip++;
		break;
	case lopCons:
		if(ctx.sp < 2){throwStackUnderflowError(c, "Cons");}
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
		if(ctx.csp < 0){throwStackUnderflowError(c, "ClosurePop");}
		ip++;
		break; }
	case lopTry:
		if(ctx.sp < 1){throwStackUnderflowError(c, "Try");}

		c->ip   = ip + lBytecodeGetOffset16(ip+1);
		c->sp   = ctx.sp;
		c->rsp  = lRootsGet();
		c->text = ops;

		c = lClosureNew(c, closureTry);
		c->exceptionHandler = ctx.valueStack[--ctx.sp];
		ctx.closureStack[++ctx.csp] = c;
		ip+=3;
		break;
	case lopApplyDynamic:
	case lopApply: {
		const int applyRSP = lRootsGet();
		const lBytecodeOp curOp = *ip;
		const int len = ip[1];
		if(ctx.sp < len){throwStackUnderflowError(c, "ApplyNew");}
		lVal *cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp = ctx.sp - len + 1;
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
		}
		lVal *res = lApply(c, cargs, fun);
		if(curOp == lopApplyDynamic){ ctx.sp--; }
		ctx.valueStack[ctx.sp-1] = res;
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
