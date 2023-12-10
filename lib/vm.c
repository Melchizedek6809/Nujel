/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#if !defined(NUJEL_USE_JUMPTABLE)
#if defined(__GNUC__)
#define NUJEL_USE_JUMPTABLE 1
#endif
#endif

/* Create a new Lambda Value */
static inline lVal lLambdaNew(lClosure *parent, lVal args, lVal body){
	lVal ret = lValAlloc(ltLambda, lClosureNew(parent, closureDefault));
	ret.vClosure->args = args;
	reqBytecodeArray(body);
	ret.vClosure->text = body.vBytecodeArr;
	ret.vClosure->ip   = ret.vClosure->text->data;
	return ret;
}

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
		if(unlikely(c == NULL)){
			break;
		}
		if(c->type == closureCall){
			ret = lCons(lValLambda(c),ret);
		}
	}
	return ret;
}

/* Evaluate ops within callingClosure after pushing args on the stack.
 *
 * Doesn't look particularly elegant at times, but gets turned into pretty
 * efficient machine code.
 */
lVal lBytecodeEval(lClosure *callingClosure, lBytecodeArray *text){
	const lBytecodeOp *ip;
	lBytecodeArray *ops = text;
	lClosure *c = callingClosure;
	lThread ctx;
	const lVal * lits = text->literals->data;
	lVal exceptionThrownValue;

#define lGarbageCollectIfNecessary() do {\
	if(unlikely(lGCShouldRunSoon)){\
		lGarbageCollect(&ctx);\
	}\
} while(0)

#ifndef NUJEL_USE_JUMPTABLE
	#define vmdispatch(o)	switch(o)
	#define vmcase(l)	case l:
	#define vmbreak	break
#else
	#define vmdispatch(x)	goto *vmJumptable[x];
	#define vmcase(label)	l##label:
	#define vmbreak	vmdispatch(*ip++);

	static const void *const vmJumptable[] = {
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
		&&llopGenSet,
		&&llopUnequalPred,
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
		const lVal a = ctx.valueStack[ctx.sp-2];
		const lVal b = ctx.valueStack[ctx.sp-1];
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
		const lVal a = ctx.valueStack[ctx.sp-2];
		const lVal b = ctx.valueStack[ctx.sp-1];
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
		const lVal a = ctx.valueStack[ctx.sp-2];
		const lVal b = ctx.valueStack[ctx.sp-1];
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
		const lVal a = ctx.valueStack[ctx.sp-2];
		const lVal b = ctx.valueStack[ctx.sp-1];
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
				const lVal r = lValFloat(a.vFloat / b.vFloat);
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(likely(b.type == ltInt)){
				const lVal r = lValFloat(a.vFloat / (float)b.vInt);
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
		const lVal a = ctx.valueStack[ctx.sp-2];
		const lVal b = ctx.valueStack[ctx.sp-1];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp-1].vInt = a.vInt % b.vInt;
			} else if(likely(b.type == ltFloat)){
				const lVal r = lValFloat(fmod(a.vInt, b.vFloat));
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
				const lVal r = lValFloat(fmod(a.vFloat, b.vFloat));
				if(unlikely(r.type == ltException)){
					exceptionThrownValue = r;
					goto throwException;
				}
				ctx.valueStack[ctx.sp-1] = r;
			} else if(likely(b.type == ltInt)){
				const lVal r = lValFloat(fmod(a.vFloat, b.vInt));
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

#define vmBinaryIntegerOp(OP) do{\
	const lVal a = ctx.valueStack[ctx.sp-2];\
	const lVal b = ctx.valueStack[ctx.sp-1];\
	ctx.sp--;\
	if(likely((a.type == ltInt) && (b.type == ltInt))){\
		ctx.valueStack[ctx.sp-1].vInt = a.vInt OP b.vInt;\
	} else {\
		exceptionThrownValue = lValExceptionNonNumeric(b);\
		goto throwException;\
	}\
	} while(0)

	vmcase(lopBitShiftLeft)
		vmBinaryIntegerOp(<<);
		vmbreak;
	vmcase(lopBitShiftRight)
		vmBinaryIntegerOp(>>);
		vmbreak;
	vmcase(lopBitAnd)
		vmBinaryIntegerOp(&);
		vmbreak;
	vmcase(lopBitOr)
		vmBinaryIntegerOp(|);
		vmbreak;
	vmcase(lopBitXor)
		vmBinaryIntegerOp(^);
		vmbreak;


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

#define vmBinaryPredicateOp(OP) do{\
	const lVal a = ctx.valueStack[ctx.sp-2];\
	const lVal b = ctx.valueStack[ctx.sp-1];\
	ctx.sp--;\
	ctx.valueStack[ctx.sp-1] = lValBool(OP);\
	} while(0)

	vmcase(lopLessPred)
		vmBinaryPredicateOp(lValGreater(a, b) < 0);
		vmbreak;
	vmcase(lopLessEqPred)
		vmBinaryPredicateOp(lValGreater(a, b) <= 0);
		vmbreak;
	vmcase(lopEqualPred)
		vmBinaryPredicateOp(lValEqual(a, b));
		vmbreak;
	vmcase(lopGreaterEqPred)
		vmBinaryPredicateOp(lValGreater(a,b) >= 0);
		vmbreak;
	vmcase(lopGreaterPred)
		vmBinaryPredicateOp(lValGreater(a, b) > 0);
		vmbreak;
	vmcase(lopUnequalPred)
		vmBinaryPredicateOp(!lValEqual(a, b));
		vmbreak;


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
		// Could be optimized like lopGetVal, but this opcode so rarely
		// used that I prefer to keep it simple
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
		const lSymbol *s = lits[*ip++].vSymbol;
		for (lClosure *cc = c; cc; cc = cc->parent) {
			lTree *t = cc->data;
			while(t){
				if(s == t->key){
					ctx.valueStack[ctx.sp++] = t->value;
					vmbreak;
				}
				t = s > t->key ? t->right : t->left;
			}
		}
		exceptionThrownValue = lValException(lSymUnboundVariable, "Can't resolve symbol", lValSymS(s));
		goto throwException; }
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
		// Could be optimized like lopSetVal, but this opcode so rarely
		// used that I prefer to keep it simple
		const uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		lSetClosureSym(c, lits[v].vSymbol, ctx.valueStack[ctx.sp-1]);
		vmbreak; }
	vmcase(lopSetVal) {
		const lSymbol *s = lits[*ip++].vSymbol;
		for (lClosure *cc = c; cc; cc = cc->parent) {
			lTree *t = cc->data;
			while(t){
				if(t->key == s){
					t->value = ctx.valueStack[ctx.sp-1];
					vmbreak;
				}
				t = s > t->key ? t->right : t->left;
			}
		}
		exceptionThrownValue = lValException(lSymUnboundVariable, "Can't set symbol", lValSymS(s));
		goto throwException; }
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
		const lVal a = ctx.valueStack[ctx.sp-1];

		if (likely(a.type == ltInt)) {
			ctx.valueStack[ctx.sp-1] = lValBool(a.vInt == 0);
		} else if(a.type == ltFloat) {
			ctx.valueStack[ctx.sp-1] = lValBool(a.vFloat == 0.0);
		} else {
			ctx.valueStack[ctx.sp-1] = lValBool(false);
		}
		vmbreak;
	}
	vmcase(lopIncInt)
		if(likely(ctx.valueStack[ctx.sp-1].type == ltInt)){
			ctx.valueStack[ctx.sp-1].vInt++;
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

		while(likely(c != NULL) && (c->type != closureTry)){
			if(unlikely(ctx.csp <= 0)){
				free(ctx.closureStack);
				free(ctx.valueStack);
				exceptionThrownValue.type = ltException;
				return exceptionThrownValue;
			}
			c = ctx.closureStack[--ctx.csp];
		}

		lVal cargs = lCons(exceptionThrownValue, NIL);
		if(unlikely(c == NULL) || (ctx.csp <= 0)){
			exit(66);
		}
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
			case  0: nv = fun.vNFunc->fp(); break;
			case  1: nv = fun.vNFunc->fpC(c); break;
			case  2: nv = fun.vNFunc->fpV(lCar(cargs)); break;
			case  3: nv = fun.vNFunc->fpCV(c, lCar(cargs)); break;
			case  4: nv = fun.vNFunc->fpVV(lCar(cargs), lCadr(cargs)); break;
			case  5: nv = fun.vNFunc->fpCVV(c, lCar(cargs), lCadr(cargs)); break;
			case  6: nv = fun.vNFunc->fpVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs)); break;
			case  7: nv = fun.vNFunc->fpCVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs)); break;
			case  8: nv = fun.vNFunc->fpVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs)); break;
			case  9: nv = fun.vNFunc->fpCVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs)); break;
			case 10: nv = fun.vNFunc->fpVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs)))); break;
			case 11: nv = fun.vNFunc->fpCVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs)))); break;
			case 12: nv = fun.vNFunc->fpVVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCadr(lCddr(lCddr(cargs)))); break;
			case 13: nv = fun.vNFunc->fpCVVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCadr(lCddr(lCddr(cargs)))); break;
			case 14: nv = fun.vNFunc->fpVVVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCaddr(lCddr(lCddr(cargs))), lCadddr(lCddr(lCddr(cargs)))); break;
			case 15: nv = fun.vNFunc->fpCVVVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCaddr(lCddr(lCddr(cargs))), lCadddr(lCddr(lCddr(cargs)))); break;
			case 16: nv = fun.vNFunc->fpR(cargs); break;
			case 17: nv = fun.vNFunc->fpCR(c, cargs); break;
			default: nv = lValException(lSymVMError, "Unsupported funcall", fun); break;
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
			const lVal self = vs[1-len];
			const lVal nfun = lMethodLookup(fun.vSymbol, self);
			if(unlikely(nfun.type == ltException)){
				exceptionThrownValue = lValException(lSymTypeError, "Unknown method", lCons(fun, lCons(lValType(&lClassList[self.type]), NIL)));
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
			case  0: v = fun.vNFunc->fp(); break;
			case  1: v = fun.vNFunc->fpC(c); break;
			case  2: v = fun.vNFunc->fpV(vs[0]); break;
			case  3: v = fun.vNFunc->fpCV(c, vs[0]); break;
			case  4: v = fun.vNFunc->fpVV(vs[-1], vs[0]); break;
			case  5: v = fun.vNFunc->fpCVV(c, vs[-1], vs[0]); break;
			case  6: v = fun.vNFunc->fpVVV(vs[-2], vs[-1], vs[0]); break;
			case  7: v = fun.vNFunc->fpCVVV(c, vs[-2], vs[-1], vs[0]); break;
			case  8: v = fun.vNFunc->fpVVVV(vs[-3], vs[-2], vs[-1], vs[0]); break;
			case  9: v = fun.vNFunc->fpCVVVV(c, vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 10: v = fun.vNFunc->fpVVVVV(vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 11: v = fun.vNFunc->fpCVVVVV(c, vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 12: v = fun.vNFunc->fpVVVVVV(vs[-5], vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 13: v = fun.vNFunc->fpCVVVVVV(c, vs[-5], vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 14: v = fun.vNFunc->fpVVVVVVV(vs[-6], vs[-5], vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 15: v = fun.vNFunc->fpCVVVVVVV(c, vs[-6], vs[-5], vs[-4], vs[-3], vs[-2], vs[-1], vs[0]); break;
			case 16: v = fun.vNFunc->fpR(lStackBuildList(ctx.valueStack, argsSp, len)); break;
			case 17: v = fun.vNFunc->fpCR(c, lStackBuildList(ctx.valueStack, argsSp, len)); break;
			default: v = lValException(lSymVMError, "Unsupported funcall", fun); break;
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
		if(unlikely(fun.type == ltKeyword)){
			lVal self = lCar(cargs);
			lVal nfun = lMethodLookup(fun.vSymbol, self);
			if(unlikely(nfun.type == ltException)){
				exceptionThrownValue = lValException(lSymTypeError, "Unknown method", lCons(fun, lCons(lValType(&lClassList[self.type]), NIL)));
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
			case  0: v = fun.vNFunc->fp(); break;
			case  1: v = fun.vNFunc->fpC(c); break;
			case  2: v = fun.vNFunc->fpV(lCar(cargs)); break;
			case  3: v = fun.vNFunc->fpCV(c, lCar(cargs)); break;
			case  4: v = fun.vNFunc->fpVV(lCar(cargs), lCadr(cargs)); break;
			case  5: v = fun.vNFunc->fpCVV(c, lCar(cargs), lCadr(cargs)); break;
			case  6: v = fun.vNFunc->fpVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs)); break;
			case  7: v = fun.vNFunc->fpCVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs)); break;
			case  8: v = fun.vNFunc->fpVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs)); break;
			case  9: v = fun.vNFunc->fpCVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs)); break;
			case 10: v = fun.vNFunc->fpVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs)))); break;
			case 11: v = fun.vNFunc->fpCVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs)))); break;
			case 12: v = fun.vNFunc->fpVVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCadr(lCddr(lCddr(cargs)))); break;
			case 13: v = fun.vNFunc->fpCVVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCadr(lCddr(lCddr(cargs)))); break;
			case 14: v = fun.vNFunc->fpVVVVVVV(lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCaddr(lCddr(lCddr(cargs))), lCadddr(lCddr(lCddr(cargs)))); break;
			case 15: v = fun.vNFunc->fpCVVVVVVV(c, lCar(cargs), lCadr(cargs), lCaddr(cargs), lCadddr(cargs), lCar(lCddr(lCddr(cargs))), lCaddr(lCddr(lCddr(cargs))), lCadddr(lCddr(lCddr(cargs)))); break;
			case 16: v = fun.vNFunc->fpR(cargs); break;
			case 17: v = fun.vNFunc->fpCR(c, cargs); break;
			default: v = lValException(lSymVMError, "Unsupported funcall", fun); break;
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
		return ret; }
	}}
}

lVal lValBytecodeArray(const lBytecodeOp *ops, int opsLength, lArray *literals){
	lVal ret = lValAlloc(ltBytecodeArr, lBytecodeArrayAlloc(opsLength));
	ret.vBytecodeArr->literals = literals;
	ret.vBytecodeArr->literals->flags |= ARRAY_IMMUTABLE;
	memcpy(ret.vBytecodeArr->data, ops, opsLength);
	return ret;
}

static lVal lnmBytecodeArrayArray(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	const int len = arr->dataEnd - arr->data;

	lVal ret = lValAlloc(ltArray, lArrayAlloc(len));
	for(int i=0;i<len;i++){
		ret.vArray->data[i] = lValInt(arr->data[i]);
	}
	return ret;
}

static lVal lnmBytecodeArrayLiterals(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	if(unlikely(arr->literals == NULL)){
		return NIL;
	} else {
		return lValAlloc(ltArray, arr->literals);
	}
}

static lVal lnmBytecodeArrayLength(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	return lValInt(arr->dataEnd - arr->data);
}

void lOperationsBytecode(){
	lClass *BytecodeArray = &lClassList[ltBytecodeArr];
	lAddNativeMethodV(BytecodeArray, lSymS("array"),    "(self)", lnmBytecodeArrayArray, 0);
	lAddNativeMethodV(BytecodeArray, lSymS("literals"), "(self)", lnmBytecodeArrayLiterals, 0);
	lAddNativeMethodV(BytecodeArray, lSymS("length"),   "(self)", lnmBytecodeArrayLength, 0);
}

/* Initialize the allocator and symbol table, needs to be called before as
 * soon as possible, since most procedures depend on it.*/
void lInit(){
	lSymbolInit();
}
