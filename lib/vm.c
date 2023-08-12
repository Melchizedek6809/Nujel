/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>

#if !defined(NUJEL_USE_JUMPTABLE)
#if defined(__GNUC__)
#define NUJEL_USE_JUMPTABLE 1
#endif
#endif

#if defined(_MSC_VER)
#define NORETURN
#else
#define NORETURN __attribute__((noreturn))
#endif

/* Read an encoded signed 16-bit offset at ip */
static inline i64 lBytecodeGetOffset16(const lBytecodeOp *ip){
	const int x = (ip[0] << 8) | ip[1];
	return (x < (1 << 15)) ? x : -((1<<16) - x);
}

/* Build a list of length len in stack starting at sp */
static lVal lStackBuildList(lVal *stack, int sp, int len){
	lVal *vsp = &stack[sp - (len)];
	lVal ret = NIL;
	for(int i = len-1; i >= 0; i--){
		ret = lCons(vsp[i], ret);
	}
	return ret;
}

static void lBytecodeEnsureSufficientStack(lThread *ctx){
	const int closureSizeLeft = (ctx->closureStackSize - ctx->csp) - 1;
	if(unlikely(closureSizeLeft < 16)){
		ctx->closureStackSize += 32;
		lClosure **t = realloc(ctx->closureStack, ctx->closureStackSize * sizeof(lClosure *));
		if(unlikely(t == NULL)){ exit(56); }
		ctx->closureStack = t;
	}

	const int valueSizeLeft = ctx->valueStackSize - ctx->sp;
	if(unlikely(valueSizeLeft < 32)){
		ctx->valueStackSize += 128;
		lVal *t = realloc(ctx->valueStack, ctx->valueStackSize * sizeof(lVal));
		if(unlikely(t == NULL)){ exit(57); }
		ctx->valueStack = t;
	}
}

static lVal stackTrace(const lThread *ctx){
	lVal ret = NIL;
	for(int i=0;i<=ctx->csp;i++){
		lClosure *c = ctx->closureStack[i];
		if(c->type == closureCall){
			ret = lCons(lValLambda(c),ret);
		}
	}
	return ret;
}

static inline void lGarbageCollectIfNecessary(){
	if(unlikely(lGCShouldRunSoon)){
		lGarbageCollect();
	}
}

#define vmdispatch(o)	switch(o)
#define vmcase(l)	case l:
#define vmbreak	break

/* Evaluate ops within callingClosure after pushing args on the stack */
lVal lBytecodeEval(lClosure *callingClosure, lBytecodeArray *text){
	const lBytecodeOp *ip;
	lBytecodeArray * ops = text;
	lClosure * c = callingClosure;
	lThread ctx;
	const lVal * lits = text->literals->data;
	lVal exceptionThrownValue;

	#ifdef NUJEL_USE_JUMPTABLE
	#undef vmdispatch
	#undef vmcase
	#undef vmbreak

	#define vmdispatch(x)	goto *vmJumptable[x];
	#define vmcase(label)	l##label:
	#define vmbreak	vmdispatch(*ip++);

	static const void *const vmJumptable[256] = {
		&&llopNOP,
		&&llopRet,
		&&llopIntByte,
		&&llopIntAdd,
		&&llopApply,
		&&llopSetVal,
		&&llopPushValExt,
		&&llopDefVal,
		&&llopDefValExt,
		&&llopJmp,
		&&llopJt,
		&&llopJf,
		&&llopDup,
		&&llopDrop,
		&&llopGetVal,
		&&llopGetValExt,
		&&llopSetValExt,
		&&llopCar,
		&&llopCdr,
		&&llopClosurePush,
		&&llopCons,
		&&llopLet,
		&&llopClosurePop,
		&&llopFnDynamic,
		&&llopMacroDynamic,
		&&llopTry,
		&&llopPushVal,
		&&llopPushTrue,
		&&llopPushFalse,
		&&llopEval,
		&&llopLessPred,
		&&llopLessEqPred,
		&&llopEqualPred,
		&&llopGreaterEqPred,
		&&llopGreaterPred,
		&&llopIncInt,
		&&llopPushNil,
		&&llopAdd,
		&&llopSub,
		&&llopMul,
		&&llopDiv,
		&&llopRem,
		&&llopZeroPred,
		&&llopRef,
		&&llopCadr,
		&&llopMutableEval,
		&&llopList,
		&&llopThrow,
		&&llopApplyCollection,
		&&llopBitShiftLeft,
		&&llopBitShiftRight,
		&&llopBitAnd,
		&&llopBitOr,
		&&llopBitXor,
		&&llopBitNot,
		&&llopGenSet
	};
	#endif

	ctx.closureStackSize = 256;
	ctx.valueStackSize   = 32;
	ctx.closureStack     = malloc(ctx.closureStackSize * sizeof(lClosure *));
	ctx.valueStack       = malloc(ctx.valueStackSize * sizeof(lVal));
	ctx.csp              = 0;
	ctx.sp               = 0;
	ctx.closureStack[0]  = c;
	ctx.text             = text;

	const int RSP = lRootsGet();
	lRootsThreadPush(&ctx);
	ip = ops->data;

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
	vmdispatch(*ip++){
	vmcase(lopNOP)
		vmbreak;
	vmcase(lopIntByte)
		ctx.valueStack[ctx.sp++] = lValInt((i8)*ip++);
		vmbreak;
	vmcase(lopAdd) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp-1].vInt += b.vInt;
			} else if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp-1] = lValFloat(b.vFloat + a.vInt);
			} else if(b.type != ltNil){
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp-1].vFloat += b.vFloat;
			} else if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp-1].vFloat += b.vInt;
			} else if(b.type != ltNil) {
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			}
		} else if(a.type != ltNil){
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		} else {
			ctx.valueStack[ctx.sp-1] = lValInt(0);
		}
		vmbreak; }
	vmcase(lopSub) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp-1].vInt -= b.vInt;
			} else if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp-1] = lValFloat(a.vInt - b.vFloat);
			} else if(b.type != ltNil){
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			} else {
				ctx.valueStack[ctx.sp-1].vInt = -ctx.valueStack[ctx.sp-1].vInt;
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp-1].vFloat -= b.vFloat;
			} else if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp-1].vFloat -= b.vInt;
			} else if(b.type != ltNil) {
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			} else {
				ctx.valueStack[ctx.sp-1].vFloat = -ctx.valueStack[ctx.sp-1].vFloat;
			}
		} else if(a.type != ltNil){
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		} else {
			exceptionThrownValue = lValExceptionArity(a, 2);
			goto throwException;
		}
		vmbreak; }
	vmcase(lopMul) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp-1].vInt *= b.vInt;
			} else if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp-1] = lValFloat(a.vInt * b.vFloat);
			} else if(b.type != ltNil){
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			} else {
				exceptionThrownValue = lValExceptionArity(a, 2);
				goto throwException;
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp-1].vFloat *= b.vFloat;
			} else if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp-1].vFloat *= b.vInt;
			} else if(b.type != ltNil) {
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			} else {
				exceptionThrownValue = lValExceptionArity(a, 2);
				goto throwException;
			}
		} else if(a.type != ltNil){
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		} else {
			ctx.valueStack[ctx.sp-1] = lValInt(1);
		}
		vmbreak; }
	vmcase(lopDiv) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				lVal r = lValFloat((float)a.vInt / (float)b.vInt);
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(likely(b.type == ltFloat)){
				lVal r = lValFloat((float)a.vInt / b.vFloat);
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(b.type != ltNil){
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			} else {
				exceptionThrownValue = lValExceptionArity(a, 2);
				goto throwException;
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				lVal r = lValFloat(a.vFloat / b.vFloat);
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(likely(b.type == ltInt)){
				lVal r = lValFloat(a.vFloat / (float)b.vInt);
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(b.type != ltNil) {
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			} else {
				exceptionThrownValue = lValExceptionArity(a, 2);
				goto throwException;
			}
		} else if(a.type != ltNil){
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		} else {
			exceptionThrownValue = lValExceptionArity(a, 2);
			goto throwException;
		}
		vmbreak; }
	vmcase(lopRem) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp-1].vInt = a.vInt % b.vInt;
			} else if(likely(b.type == ltFloat)){
				lVal r = lValFloat(fmod(a.vInt, b.vFloat));
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(unlikely(b.type != ltNil)){
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				lVal r = lValFloat(fmod(a.vFloat, b.vFloat));
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(likely(b.type == ltInt)){
				lVal r = lValFloat(fmod(a.vFloat, b.vInt));
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(unlikely(b.type != ltNil)) {
				exceptionThrownValue = lValExceptionNonNumeric(b);
				goto throwException;
			}
		} else if(unlikely(a.type != ltNil)){
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		}
		vmbreak; }
	vmcase(lopBitShiftLeft) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely((a.type == ltInt) && (b.type == ltInt))){
			ctx.valueStack[ctx.sp-1].vInt = a.vInt << b.vInt;
		} else {
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		}
		vmbreak; }
	vmcase(lopBitShiftRight) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely((a.type == ltInt) && (b.type == ltInt))){
			ctx.valueStack[ctx.sp-1].vInt = a.vInt >> b.vInt;
		} else {
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		}
		vmbreak; }
	vmcase(lopBitAnd) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely((a.type == ltInt) && (b.type == ltInt))){
			ctx.valueStack[ctx.sp-1].vInt = a.vInt & b.vInt;
		} else {
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		}
		vmbreak; }
	vmcase(lopBitOr) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely((a.type == ltInt) && (b.type == ltInt))){
			ctx.valueStack[ctx.sp-1].vInt = a.vInt | b.vInt;
		} else {
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		}
		vmbreak; }
	vmcase(lopBitXor) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely((a.type == ltInt) && (b.type == ltInt))){
			ctx.valueStack[ctx.sp-1].vInt = a.vInt ^ b.vInt;
		} else {
			exceptionThrownValue = lValExceptionNonNumeric(b);
			goto throwException;
		}
		vmbreak; }
	vmcase(lopBitNot)
		if(likely(ctx.valueStack[ctx.sp-1].type == ltInt)){
			ctx.valueStack[ctx.sp-1].vInt = ~ctx.valueStack[ctx.sp-1].vInt;
		} else {
			exceptionThrownValue = lValExceptionNonNumeric(ctx.valueStack[ctx.sp-1]);
			goto throwException;
		}
		vmbreak;
	vmcase(lopIntAdd)
		ctx.valueStack[ctx.sp-2].vInt += ctx.valueStack[ctx.sp-1].vInt;
		ctx.sp--;
		vmbreak;
	vmcase(lopCons)
		ctx.valueStack[ctx.sp-2] = lCons(ctx.valueStack[ctx.sp-2], ctx.valueStack[ctx.sp-1]);
		ctx.sp--;
		vmbreak;
	vmcase(lopLessPred) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValGreater(a, b) < 0);
		ctx.sp--;
		vmbreak; }
	vmcase(lopLessEqPred) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValGreater(a, b) <= 0);
		ctx.sp--;
		vmbreak; }
	vmcase(lopEqualPred) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValEqual(a, b));
		ctx.sp--;
		vmbreak; }
	vmcase(lopGreaterEqPred) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValGreater(a,b) >= 0);
		ctx.sp--;
		vmbreak; }
	vmcase(lopGreaterPred) {
		lVal a = ctx.valueStack[ctx.sp-2];
		lVal b = ctx.valueStack[ctx.sp-1];
		ctx.valueStack[ctx.sp-2] = lValBool(lValGreater(a, b) > 0);
		ctx.sp--;
		vmbreak; }
	vmcase(lopPushNil)
		ctx.valueStack[ctx.sp++] = NIL;
		vmbreak;
	vmcase(lopPushTrue)
		ctx.valueStack[ctx.sp++] = lValBool(true);
		vmbreak;
	vmcase(lopPushFalse)
		ctx.valueStack[ctx.sp++] = lValBool(false);
		vmbreak;
	vmcase(lopPushValExt) {
		const uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		ctx.valueStack[ctx.sp++] = lits[v];
		vmbreak; }
	vmcase(lopPushVal) {
		const uint v = *ip++;
		ctx.valueStack[ctx.sp++] = lits[v];
		vmbreak; }
	vmcase(lopDup)
		ctx.sp++;
		ctx.valueStack[ctx.sp-1] = ctx.valueStack[ctx.sp-2];
		vmbreak;
	vmcase(lopDrop)
		ctx.sp--;
		vmbreak;
	vmcase(lopJmp)
		lGarbageCollectIfNecessary();
		ip += lBytecodeGetOffset16(ip)-1;
		vmbreak;
	vmcase(lopJt)
		lGarbageCollectIfNecessary();
		ip +=  castToBool(ctx.valueStack[--ctx.sp]) ? lBytecodeGetOffset16(ip)-1 : 2;
		vmbreak;
	vmcase(lopJf)
		lGarbageCollectIfNecessary();
		ip += !castToBool(ctx.valueStack[--ctx.sp]) ? lBytecodeGetOffset16(ip)-1 : 2;
		vmbreak;
	vmcase(lopDefValExt) {
		const uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		lDefineClosureSym(c, lits[v].vSymbol, ctx.valueStack[ctx.sp-1]);
		vmbreak; }
	vmcase(lopDefVal) {
		const uint v = *ip++;
		lDefineClosureSym(c, lits[v].vSymbol, ctx.valueStack[ctx.sp-1]);
		vmbreak; }
	vmcase(lopGetValExt) {
		const uint off = (ip[0] << 8) | (ip[1]);
		ip += 2;
		lVal v = lGetClosureSym(c, lits[off].vSymbol);
		if(unlikely(v.type == ltException)){
			exceptionThrownValue = v;
			goto throwException;
		}
		ctx.valueStack[ctx.sp++] = v;
		vmbreak; }
	vmcase(lopGetVal) {
		const uint off = *ip++;
		lVal v = lGetClosureSym(c, lits[off].vSymbol);
		if(unlikely(v.type == ltException)){
			exceptionThrownValue = v;
			goto throwException;
		}
		ctx.valueStack[ctx.sp++] = v;
		vmbreak; }
	vmcase(lopRef) {
		lVal v = lGenericRef(ctx.valueStack[ctx.sp-2], ctx.valueStack[ctx.sp-1]);
		if(unlikely(v.type == ltException)){
			exceptionThrownValue = v;
			goto throwException;
		}
		ctx.valueStack[ctx.sp-2] = v;
		ctx.sp--;
		vmbreak; }
	vmcase(lopSetValExt) {
		const uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		lSetClosureSym(c, lits[v].vSymbol, ctx.valueStack[ctx.sp-1]);
		vmbreak; }
	vmcase(lopSetVal) {
		const uint v = *ip++;
		lSetClosureSym(c, lits[v].vSymbol, ctx.valueStack[ctx.sp-1]);
		vmbreak; }
	vmcase(lopGenSet) {
		lVal val = ctx.valueStack[ctx.sp-1];
		lVal key = ctx.valueStack[ctx.sp-2];
		lVal col = ctx.valueStack[ctx.sp-3];
		lVal ret = lGenericSet(col, key, val);
		if(unlikely(ret.type == ltException)){
			exceptionThrownValue = ret;
			goto throwException;
		}
		ctx.sp -= 2;
		ctx.valueStack[ctx.sp-1] = ret;
		vmbreak; }
	vmcase(lopZeroPred) {
		lVal a = ctx.valueStack[ctx.sp-1];
		bool p = false;

		if(likely(a.type == ltInt)){
			p = a.vInt == 0;
		}else if(a.type == ltFloat){
			p = a.vFloat == 0.0;
		}

		ctx.valueStack[ctx.sp-1] = lValBool(p);
		vmbreak; }
	vmcase(lopIncInt)
		if(likely(ctx.valueStack[ctx.sp-1].type == ltInt)){
			ctx.valueStack[ctx.sp-1] = lValInt(ctx.valueStack[ctx.sp-1].vInt + 1);
		}
		vmbreak;
	vmcase(lopCar)
		ctx.valueStack[ctx.sp-1] = lCar(ctx.valueStack[ctx.sp-1]);
		vmbreak;
	vmcase(lopCdr)
		ctx.valueStack[ctx.sp-1] = lCdr(ctx.valueStack[ctx.sp-1]);
		vmbreak;
	vmcase(lopCadr)
		ctx.valueStack[ctx.sp-1] = lCadr(ctx.valueStack[ctx.sp-1]);
		vmbreak;
	vmcase(lopList) {
		int len = *ip++;
		lVal cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp = ctx.sp - len;
		ctx.valueStack[ctx.sp++] = cargs;
		vmbreak; }
	vmcase(lopClosurePush)
		ctx.valueStack[ctx.sp++] = lValEnvironment(c);
		vmbreak;
	vmcase(lopLet)
		c = lClosureNew(c, closureLet);
		c->type = closureLet;
		ctx.closureStack[++ctx.csp] = c;
		vmbreak;
	vmcase(lopClosurePop)
		c = ctx.closureStack[--ctx.csp];
		vmbreak;
	vmcase(lopTry)
		c->ip	= ip + lBytecodeGetOffset16(ip)-1;
		c->sp	= ctx.sp;
		c->text = ops;

		c = lClosureNew(c, closureTry);
		c->exceptionHandler = ctx.valueStack[--ctx.sp];
		ctx.closureStack[++ctx.csp] = c;
		ip+=2;
		vmbreak;
	vmcase(lopThrow) {
		lVal v = ctx.valueStack[ctx.sp-1];
		if(likely(v.type == ltPair)){
			v.type = ltException;
		}
		exceptionThrownValue = v;
	throwException:
		if(unlikely((exceptionThrownValue.type != ltPair) && (exceptionThrownValue.type != ltException))){
			exceptionThrownValue = lCons(exceptionThrownValue, NIL);
		}

		if(likely(exceptionThrownValue.type == ltException)){
			exceptionThrownValue.type = ltPair;
			lPair *t = exceptionThrownValue.vList;
			while(t->cdr.type == ltPair){
				t=t->cdr.vList;
			}
			t->cdr = lCons(stackTrace(&ctx),NIL);
		}

		while(c->type != closureTry){
			if(unlikely(ctx.csp <= 0)){
				free(ctx.closureStack);
				free(ctx.valueStack);
				lRootsRet(RSP);
				exceptionThrownValue.type = ltException;
				return exceptionThrownValue;
			}
			c = ctx.closureStack[--ctx.csp];
		}

		lVal cargs = lCons(exceptionThrownValue, NIL);
		lVal fun = c->exceptionHandler;
		c = ctx.closureStack[--ctx.csp];
		switch(fun.type){
		case ltLambda:
			c = ctx.closureStack[++ctx.csp] = lClosureNewFunCall(cargs, fun);
			ip = c->ip;
			ctx.text = ops = c->text;
			lits = ops->literals->data;
			lBytecodeEnsureSufficientStack(&ctx);
			lGarbageCollectIfNecessary();
			break;
		case ltNativeFunc: {
			lVal nv;
			switch(fun.vNFunc->argCount){
			case 0: nv = fun.vNFunc->fp(); break;
			case 1: nv = fun.vNFunc->fpC(c); break;
			case 2: nv = fun.vNFunc->fpV(lCar(cargs)); break;
			case 3: nv = fun.vNFunc->fpCV(c, lCar(cargs)); break;
			case 4: nv = fun.vNFunc->fpVV(lCar(cargs), lCadr(cargs)); break;
			case 5: nv = fun.vNFunc->fpCVV(c, lCar(cargs), lCadr(cargs)); break;
			case 6: nv = fun.vNFunc->fpVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs)); break;
			case 7: nv = fun.vNFunc->fpCVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs)); break;
			case 8: nv = fun.vNFunc->fpVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs)); break;
			case 9: nv = fun.vNFunc->fpCVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs)); break;
			case 10: nv = fun.vNFunc->fpVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs)))); break;
			case 11: nv = fun.vNFunc->fpCVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs)))); break;
			case 12: nv = fun.vNFunc->fpVVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCadr(lCddr(lCddr(cargs)))); break;
			case 13: nv = fun.vNFunc->fpCVVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCadr(lCddr(lCddr(cargs)))); break;
			case 14: nv = fun.vNFunc->fpVVVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCaddr(lCddr(lCddr(cargs))), lCadddr(lCddr(lCddr(cargs)))); break;
			case 15: nv = fun.vNFunc->fpCVVVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCaddr(lCddr(lCddr(cargs))), lCadddr(lCddr(lCddr(cargs)))); break;
			case 16: nv = fun.vNFunc->fpR(cargs); break;
			case 17: nv = fun.vNFunc->fpCR(c, cargs); break;
			default:
				exceptionThrownValue = lValException(lSymVMError, "Unsupported funcall", fun);
				goto throwException;
			}
			if(unlikely(nv.type == ltException)){
				exceptionThrownValue = nv;
				goto throwException;
			}
			ops = c->text;
			lits = ops->literals->data;
			ip = c->ip;
			ctx.text = ops;
			ctx.sp = c->sp;
			ctx.valueStack[ctx.sp++] = nv;
			break; }
		default: {
			exceptionThrownValue = lValException(lSymTypeError, "Can't apply to following val", fun);
			goto throwException; }
		}
		vmbreak; }
	vmcase(lopMacroDynamic)
	vmcase(lopFnDynamic) {
		const lBytecodeOp curOp = ip[-1];
		lVal cBody = ctx.valueStack[--ctx.sp];
		lVal cDocs = ctx.valueStack[--ctx.sp];
		lVal cArgs = ctx.valueStack[--ctx.sp];
		lVal fun = lLambdaNew(c, cArgs, cBody);
		lClosureSetMeta(fun.vClosure, cDocs);
		if(unlikely(curOp == lopMacroDynamic)){
			fun.type = ltMacro;
		}
		ctx.valueStack[ctx.sp++] = fun;
		vmbreak; }
	vmcase(lopMutableEval)
	vmcase(lopEval) {
		const lBytecodeOp curOp = ip[-1];
		lVal env = ctx.valueStack[--ctx.sp];
		lVal bc = ctx.valueStack[--ctx.sp];
		if(unlikely((env.type != ltEnvironment) || (bc.type != ltBytecodeArr))){
			exceptionThrownValue = lValException(lSymTypeError, "Can't eval in that", env);
			goto throwException;
		}

		c->text = ops;
		c->sp   = ctx.sp;
		c->ip   = ip;

		if(unlikely(curOp == lopMutableEval)){
			c = ctx.closureStack[++ctx.csp] = env.vClosure;
		} else {
			c = ctx.closureStack[++ctx.csp] = lClosureNew(env.vClosure, closureCall);
		}

		c->text = bc.vBytecodeArr;
		ip = c->ip = c->text->data;
		ctx.text = ops = c->text;
		lits = ops->literals->data;
		lBytecodeEnsureSufficientStack(&ctx);
		lGarbageCollectIfNecessary();

		vmbreak; }
	vmcase(lopApply) {
		const int len = *ip++;
		const int argsSp = ctx.sp;
		ctx.sp = ctx.sp - len;
		lVal fun = ctx.valueStack[--ctx.sp];
		lVal *vs = &ctx.valueStack[argsSp-1];
		if(unlikely(fun.type == ltKeyword)){
			lVal self = vs[1-len];
			lVal nfun = lMethodLookup(fun.vSymbol, self);
			if(unlikely(nfun.type == ltException)){
				exceptionThrownValue = lValException(lSymTypeError, "Unknown method", fun);
				goto throwException;
			}
			fun = nfun;
		}
		switch(fun.type){
		case ltMacro:
		case ltLambda:
			c->text = ops;
			c->sp   = ctx.sp;
			c->ip   = ip;

			ctx.closureStack[++ctx.csp] = lClosureNewFunCall(lStackBuildList(ctx.valueStack, argsSp, len), fun);
			c = ctx.closureStack[ctx.csp];
			ip = c->ip;
			ctx.text = ops = c->text;
			lits = ops->literals->data;
			lBytecodeEnsureSufficientStack(&ctx);
			lGarbageCollectIfNecessary();
			break;
		case ltNativeFunc: {
			lVal v;
			const int ac = ((fun.vNFunc->argCount >> 1)&7);
			if(unlikely(len != ac)){
				const int m = ac - len;
				vs+= m;
				if(len < ac){
					for(int i=0;i<m;i++){
						vs[-i]=NIL;
					}
				}
			}
			switch(fun.vNFunc->argCount){
			case 0: v = fun.vNFunc->fp(); break;
			case 1: v = fun.vNFunc->fpC(c); break;
			case 2: v = fun.vNFunc->fpV(vs[0]); break;
			case 3: v = fun.vNFunc->fpCV(c, vs[0]); break;
			case 4: v = fun.vNFunc->fpVV(vs[-1], vs[0]); break;
			case 5: v = fun.vNFunc->fpCVV(c, vs[-1], vs[0]); break;
			case 6: v = fun.vNFunc->fpVVV(vs[-2], vs[-1], vs[0]); break;
			case 7: v = fun.vNFunc->fpCVVV(c, vs[-2], vs[-1], vs[0]); break;
			case 8: v = fun.vNFunc->fpVVVV(vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 9: v = fun.vNFunc->fpCVVVV(c, vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 10: v = fun.vNFunc->fpVVVVV(vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 11: v = fun.vNFunc->fpCVVVVV(c, vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 12: v = fun.vNFunc->fpVVVVVV(vs[-5], vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 13: v = fun.vNFunc->fpCVVVVVV(c, vs[-5], vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 14: v = fun.vNFunc->fpVVVVVVV(vs[-6], vs[-5], vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 15: v = fun.vNFunc->fpCVVVVVVV(c, vs[-6], vs[-5], vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 16: v = fun.vNFunc->fpR(lStackBuildList(ctx.valueStack, argsSp, len)); break;
			case 17: v = fun.vNFunc->fpCR(c, lStackBuildList(ctx.valueStack, argsSp, len)); break;
			default:
				exceptionThrownValue = lValException(lSymVMError, "Unsupported funcall", fun);
				goto throwException;
			}
			if(unlikely(v.type == ltException)){
				exceptionThrownValue = v;
				goto throwException;
			}
			ctx.valueStack[ctx.sp++] = v;
			break; }
		default: {
			exceptionThrownValue = lValException(lSymTypeError, "Can't apply to following val", fun);
			goto throwException; }
		}
		vmbreak; }
	vmcase(lopApplyCollection) {
		lVal cargs = ctx.valueStack[--ctx.sp];
		lVal fun = ctx.valueStack[--ctx.sp];
		switch(fun.type){
		case ltMacro:
		case ltLambda:
			c->text = ops;
			c->sp   = ctx.sp;
			c->ip   = ip;

			ctx.closureStack[++ctx.csp] = lClosureNewFunCall(cargs, fun);
			c = ctx.closureStack[ctx.csp];
			ip = c->ip;
			ctx.text = ops = c->text;
			lits = ops->literals->data;
			lBytecodeEnsureSufficientStack(&ctx);
			lGarbageCollectIfNecessary();
			break;
		case ltNativeFunc: {
			lVal v;
			switch(fun.vNFunc->argCount){
			case 0: v = fun.vNFunc->fp(); break;
			case 1: v = fun.vNFunc->fpC(c); break;
			case 2: v = fun.vNFunc->fpV(lCar(cargs)); break;
			case 3: v = fun.vNFunc->fpCV(c, lCar(cargs)); break;
			case 4: v = fun.vNFunc->fpVV(lCar(cargs), lCadr(cargs)); break;
			case 5: v = fun.vNFunc->fpCVV(c, lCar(cargs), lCadr(cargs)); break;
			case 6: v = fun.vNFunc->fpVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs)); break;
			case 7: v = fun.vNFunc->fpCVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs)); break;
			case 8: v = fun.vNFunc->fpVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs)); break;
			case 9: v = fun.vNFunc->fpCVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs)); break;
			case 10: v = fun.vNFunc->fpVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs)))); break;
			case 11: v = fun.vNFunc->fpCVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs)))); break;
			case 12: v = fun.vNFunc->fpVVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCadr(lCddr(lCddr(cargs)))); break;
			case 13: v = fun.vNFunc->fpCVVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCadr(lCddr(lCddr(cargs)))); break;
			case 14: v = fun.vNFunc->fpVVVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCaddr(lCddr(lCddr(cargs))), lCadddr(lCddr(lCddr(cargs)))); break;
			case 15: v = fun.vNFunc->fpCVVVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCaddr(lCddr(lCddr(cargs))), lCadddr(lCddr(lCddr(cargs)))); break;
			case 16: v = fun.vNFunc->fpR(cargs); break;
			case 17: v = fun.vNFunc->fpCR(c, cargs); break;
			default:
				exceptionThrownValue = lValException(lSymVMError, "Unsupported funcall", fun);
				goto throwException;
			}
			if(unlikely(v.type == ltException)){
				exceptionThrownValue = v;
				goto throwException;
			}
			ctx.valueStack[ctx.sp++] = v;
			break; }
		default: {
			exceptionThrownValue = lValException(lSymTypeError, "Can't apply to following val", fun);
			goto throwException; }
		}
		vmbreak; }
	vmcase(lopRet)
		if(likely(ctx.csp > 0)){
			while(ctx.closureStack[ctx.csp]->type != closureCall){
				if(unlikely(--ctx.csp <= 0)){goto topLevelReturn;}
			}
			lVal ret = ctx.valueStack[ctx.sp-1];
			c   = ctx.closureStack[--ctx.csp];
			ip  = c->ip;
			ops = c->text;
			lits = ops->literals->data;
			ctx.text = ops;
			ctx.sp = c->sp;
			ctx.valueStack[ctx.sp++] = ret;
			vmbreak;
		}
	topLevelReturn: {
		lVal ret = ctx.valueStack[ctx.sp-1];
		free(ctx.closureStack);
		free(ctx.valueStack);
		lRootsRet(RSP);
		return ret; }
	}}
}
