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

static inline void lClosureSetMeta(lClosure *c, lVal doc){
	if(unlikely(doc.type != ltTree)){
		return;
	}
	lTree *t = doc.vTree->root;
	c->meta = (t && t->flags & TREE_IMMUTABLE) ? lTreeDup(t) : t;
}

/* Create a new Lambda Value */
static inline lVal lLambdaNew(lClosure *parent, lVal args, lVal body){
	reqBytecodeArray(body);
	lVal ret = lValAlloc(ltLambda, lClosureNew(parent, closureDefault));
	ret.vClosure->args = args;
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
	for(int i = len; i > 0; i--){
		ret = lCons(vsp[i], ret);
	}
	return ret;
}

static void lBytecodeEnsureSufficientStack(lThread *ctx){
	const int closureSizeLeft = (ctx->closureStackSize - ctx->csp) - 1;
	if(unlikely(closureSizeLeft < 16)){
		ctx->closureStackSize += 512;
		lClosure **t = realloc(ctx->closureStack, ctx->closureStackSize * sizeof(lClosure *));
		if(unlikely(t == NULL)){ exit(56); }
		ctx->closureStack = t;
	}

	const int valueSizeLeft = ctx->valueStackSize - ctx->sp;
	if(unlikely(valueSizeLeft < 32)){
		ctx->valueStackSize += 512;
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

static inline lClosure *funCallClosure(lVal lambda) {
	lClosure *tmpc = lClosureAllocRaw();
	tmpc->parent = lambda.vClosure;
	tmpc->type   = closureCall;
	tmpc->text   = lambda.vClosure->text;
	tmpc->ip     = tmpc->text->data;
	return tmpc;
}

#define lStoreInClosure(ipOffset) do {\
	c->ip = ip + (ipOffset);\
	c->sp = ctx.sp;\
	c->text = ctx.text = ops;\
} while (0)

#define lRestoreFromClosure() do {\
	ip = c->ip;\
	ctx.text = ops = c->text;\
	lits = ops->literals->data;\
} while (0)

#define lGarbageCollectIfNecessary() do {\
	if(unlikely(lGCShouldRunSoon)){\
		lGarbageCollect(&ctx);\
	}\
} while(0)

#define lThrow(v) do {\
	exceptionThrownValue = v;\
	goto throwException;\
} while(0)

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
	ctx.closureStackSize = 512;
	ctx.valueStackSize   = 512;
	ctx.closureStack     = malloc(ctx.closureStackSize * sizeof(lClosure *));
	ctx.valueStack       = malloc(ctx.valueStackSize * sizeof(lVal));
	ctx.csp              = 0;
	ctx.sp              = 0;
	ctx.closureStack[0]  = c;
	ctx.text             = text;
	ctx.valueStack[0]    = NIL;
	ctx.valueStack[1]    = NIL;

	ip = ops->data;

	while(true){
	dispatchLoop:
	vmdispatch(*ip++){
	vmcase(lopNOP)
		vmbreak;
	vmcase(lopIntByte)
		ctx.valueStack[++ctx.sp] = lValInt((i8)*ip++);
		vmbreak;
	vmcase(lopAdd) {
		const lVal a = ctx.valueStack[ctx.sp-1];
		const lVal b = ctx.valueStack[ctx.sp];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp].vInt += b.vInt;
			} else if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp] = lValFloat(b.vFloat + a.vInt);
			} else if(b.type != ltNil){
				lThrow(lValExceptionNonNumeric(b));
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp].vFloat += b.vFloat;
			} else if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp].vFloat += b.vInt;
			} else if(b.type != ltNil) {
				lThrow(lValExceptionNonNumeric(b));
			}
		} else if(a.type != ltNil){
			lThrow(lValExceptionNonNumeric(b));
		} else {
			ctx.valueStack[ctx.sp] = lValInt(0);
		}
		vmbreak; }
	vmcase(lopSub) {
		const lVal a = ctx.valueStack[ctx.sp-1];
		const lVal b = ctx.valueStack[ctx.sp];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp].vInt -= b.vInt;
			} else if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp] = lValFloat(a.vInt - b.vFloat);
			} else if(b.type != ltNil){
				lThrow(lValExceptionNonNumeric(b));
			} else {
				ctx.valueStack[ctx.sp].vInt = -ctx.valueStack[ctx.sp].vInt;
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp].vFloat -= b.vFloat;
			} else if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp].vFloat -= b.vInt;
			} else if(b.type != ltNil) {
				lThrow(lValExceptionNonNumeric(b));
			} else {
				ctx.valueStack[ctx.sp].vFloat = -ctx.valueStack[ctx.sp].vFloat;
			}
		} else if(a.type != ltNil){
			lThrow(lValExceptionNonNumeric(b));
		} else {
			lThrow(lValExceptionArity(a, 2));
		}
		vmbreak; }
	vmcase(lopMul) {
		const lVal a = ctx.valueStack[ctx.sp-1];
		const lVal b = ctx.valueStack[ctx.sp];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp].vInt *= b.vInt;
			} else if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp] = lValFloat(a.vInt * b.vFloat);
			} else if(b.type != ltNil){
				lThrow(lValExceptionNonNumeric(b));
			} else {
				lThrow(lValExceptionArity(a, 2));
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				ctx.valueStack[ctx.sp].vFloat *= b.vFloat;
			} else if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp].vFloat *= b.vInt;
			} else if(b.type != ltNil) {
				lThrow(lValExceptionNonNumeric(b));
			} else {
				lThrow(lValExceptionArity(a, 2));
			}
		} else if(a.type != ltNil){
			lThrow(lValExceptionNonNumeric(b));
		} else {
			ctx.valueStack[ctx.sp] = lValInt(1);
		}
		vmbreak; }
	vmcase(lopDiv) {
		const lVal a = ctx.valueStack[ctx.sp-1];
		const lVal b = ctx.valueStack[ctx.sp];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				lVal r = lValFloat((float)a.vInt / (float)b.vInt);
				if(unlikely(r.type == ltException)){
					lThrow(r);
				}
				ctx.valueStack[ctx.sp] = r;
			} else if(likely(b.type == ltFloat)){
				lVal r = lValFloat((float)a.vInt / b.vFloat);
				if(unlikely(r.type == ltException)){
					lThrow(r);
				}
				ctx.valueStack[ctx.sp] = r;
			} else if(b.type != ltNil){
				lThrow(lValExceptionNonNumeric(b));
			} else {
				lThrow(lValExceptionArity(a, 2));
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				const lVal r = lValFloat(a.vFloat / b.vFloat);
				if(unlikely(r.type == ltException)){
					lThrow(r);
				}
				ctx.valueStack[ctx.sp] = r;
			} else if(likely(b.type == ltInt)){
				const lVal r = lValFloat(a.vFloat / (float)b.vInt);
				if(unlikely(r.type == ltException)){
					lThrow(r);
				}
				ctx.valueStack[ctx.sp] = r;
			} else if(b.type != ltNil) {
				lThrow(lValExceptionNonNumeric(b));
			} else {
				lThrow(lValExceptionArity(a, 2));
			}
		} else if(a.type != ltNil){
			lThrow(lValExceptionNonNumeric(b));
		} else {
			lThrow(lValExceptionArity(a, 2));
		}
		vmbreak; }
	vmcase(lopRem) {
		const lVal a = ctx.valueStack[ctx.sp-1];
		const lVal b = ctx.valueStack[ctx.sp];
		ctx.sp--;
		if(likely(a.type == ltInt)){
			if(likely(b.type == ltInt)){
				ctx.valueStack[ctx.sp].vInt = a.vInt % b.vInt;
			} else if(likely(b.type == ltFloat)){
				const lVal r = lValFloat(fmod(a.vInt, b.vFloat));
				if(unlikely(r.type == ltException)){
					lThrow(r);
				}
				ctx.valueStack[ctx.sp] = r;
			} else if(unlikely(b.type != ltNil)){
				lThrow(lValExceptionNonNumeric(b));
			}
		} else if(likely(a.type == ltFloat)){
			if(likely(b.type == ltFloat)){
				const lVal r = lValFloat(fmod(a.vFloat, b.vFloat));
				if(unlikely(r.type == ltException)){
					lThrow(r);
				}
				ctx.valueStack[ctx.sp] = r;
			} else if(likely(b.type == ltInt)){
				const lVal r = lValFloat(fmod(a.vFloat, b.vInt));
				if(unlikely(r.type == ltException)){
					lThrow(r);
				}
				ctx.valueStack[ctx.sp] = r;
			} else if(unlikely(b.type != ltNil)) {
				lThrow(lValExceptionNonNumeric(b));
			}
		} else if(unlikely(a.type != ltNil)){
			lThrow(lValExceptionNonNumeric(b));
		}
		vmbreak; }

#define vmBinaryIntegerOp(OP) do{\
	const lVal a = ctx.valueStack[ctx.sp-1];\
	const lVal b = ctx.valueStack[ctx.sp];\
	ctx.sp--;\
	if(likely((a.type == ltInt) && (b.type == ltInt))){\
		ctx.valueStack[ctx.sp].vInt = a.vInt OP b.vInt;\
	} else {\
		lThrow(lValExceptionNonNumeric(b));\
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
		if(likely(ctx.valueStack[ctx.sp].type == ltInt)){
			ctx.valueStack[ctx.sp].vInt = ~ctx.valueStack[ctx.sp].vInt;
		} else {
			lThrow(lValExceptionNonNumeric(ctx.valueStack[ctx.sp]));
		}
		vmbreak;
	vmcase(lopIntAdd)
		if(likely((ctx.valueStack[ctx.sp-1].type == ltInt) && (ctx.valueStack[ctx.sp].type == ltInt))){
			ctx.valueStack[ctx.sp-1].vInt += ctx.valueStack[ctx.sp].vInt;
			ctx.sp--;
		} else {
			lThrow(lValExceptionNonNumeric(ctx.valueStack[ctx.sp]));
		}
		vmbreak;
	vmcase(lopCons)
		ctx.valueStack[ctx.sp-1] = lCons(ctx.valueStack[ctx.sp-1], ctx.valueStack[ctx.sp]);
		ctx.sp--;
		vmbreak;

#define vmBinaryPredicateOp(OP) do{\
	const lVal a = ctx.valueStack[ctx.sp-1];\
	const lVal b = ctx.valueStack[ctx.sp];\
	ctx.sp--;\
	ctx.valueStack[ctx.sp] = lValBool(OP);\
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
		ctx.valueStack[++ctx.sp] = NIL;
		vmbreak;
	vmcase(lopPushTrue)
		ctx.valueStack[++ctx.sp] = lValBool(true);
		vmbreak;
	vmcase(lopPushFalse)
		ctx.valueStack[++ctx.sp] = lValBool(false);
		vmbreak;
	vmcase(lopPushValExt) {
		const uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		ctx.valueStack[++ctx.sp] = lits[v];
		vmbreak; }
	vmcase(lopPushVal) {
		const uint v = *ip++;
		ctx.valueStack[++ctx.sp] = lits[v];
		vmbreak; }
	vmcase(lopDup)
		ctx.sp++;
		ctx.valueStack[ctx.sp] = ctx.valueStack[ctx.sp-1];
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
		ip +=  castToBool(ctx.valueStack[ctx.sp--]) ? lBytecodeGetOffset16(ip)-1 : 2;
		vmbreak;
	vmcase(lopJf)
		lGarbageCollectIfNecessary();
		ip += !castToBool(ctx.valueStack[ctx.sp--]) ? lBytecodeGetOffset16(ip)-1 : 2;
		vmbreak;
	vmcase(lopDefValExt) {
		const uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		lDefineClosureSym(c, lits[v].vSymbol, ctx.valueStack[ctx.sp]);
		vmbreak; }
	vmcase(lopDefVal) {
		const uint v = *ip++;
		lDefineClosureSym(c, lits[v].vSymbol, ctx.valueStack[ctx.sp]);
		vmbreak; }
	vmcase(lopGetValExt) {
		// Could be optimized like lopGetVal, but this opcode so rarely
		// used that I prefer to keep it simple
		const uint off = (ip[0] << 8) | (ip[1]);
		ip += 2;
		lVal v = lGetClosureSym(c, lits[off].vSymbol);
		if(unlikely(v.type == ltException)){
			lThrow(v);
		}
		ctx.valueStack[++ctx.sp] = v;
		vmbreak; }
	vmcase(lopGetVal) {
		const lSymbol *s = lits[*ip++].vSymbol;
		for (lClosure *cc = c; cc; cc = cc->parent) {
			lTree *t = cc->data;
			while(t){
				if(s == t->key){
					ctx.valueStack[++ctx.sp] = t->value;
					goto dispatchLoop;
				}
				t = s > t->key ? t->right : t->left;
			}
		}
		lThrow(lValException(lSymUnboundVariable, "Can't resolve symbol", lValSymS(s))); }
	vmcase(lopRef) {
		lVal v = lGenericRef(ctx.valueStack[ctx.sp-1], ctx.valueStack[ctx.sp]);
		if(unlikely(v.type == ltException)){
			lThrow(v);
		}
		ctx.valueStack[--ctx.sp] = v;
		vmbreak; }
	vmcase(lopSetValExt) {
		// Could be optimized like lopSetVal, but this opcode so rarely
		// used that I prefer to keep it simple
		const uint v = (ip[0] << 8) | (ip[1]);
		ip += 2;
		lSetClosureSym(c, lits[v].vSymbol, ctx.valueStack[ctx.sp]);
		vmbreak; }
	vmcase(lopSetVal) {
		const lSymbol *s = lits[*ip++].vSymbol;
		for (lClosure *cc = c; cc; cc = cc->parent) {
			lTree *t = cc->data;
			while(t){
				if(t->key == s){
					t->value = ctx.valueStack[ctx.sp];
					goto dispatchLoop;
				}
				t = s > t->key ? t->right : t->left;
			}
		}
		lThrow(lValException(lSymUnboundVariable, "Can't set symbol", lValSymS(s))); }
	vmcase(lopGenSet) {
		lVal val = ctx.valueStack[ctx.sp];
		lVal key = ctx.valueStack[ctx.sp-1];
		lVal col = ctx.valueStack[ctx.sp-2];
		lVal ret = lGenericSet(col, key, val);
		if(unlikely(ret.type == ltException)){
			lThrow(ret);
		}
		ctx.sp -= 2;
		ctx.valueStack[ctx.sp] = ret;
		vmbreak; }
	vmcase(lopZeroPred) {
		const lVal a = ctx.valueStack[ctx.sp];

		if (likely(a.type == ltInt)) {
			ctx.valueStack[ctx.sp] = lValBool(a.vInt == 0);
		} else if(a.type == ltFloat) {
			ctx.valueStack[ctx.sp] = lValBool(a.vFloat == 0.0);
		} else {
			ctx.valueStack[ctx.sp] = lValBool(false);
		}
		vmbreak;
	}
	vmcase(lopIncInt)
		if(likely(ctx.valueStack[ctx.sp].type == ltInt)){
			ctx.valueStack[ctx.sp].vInt++;
		}
		vmbreak;
	vmcase(lopCar)
		ctx.valueStack[ctx.sp] = lCar(ctx.valueStack[ctx.sp]);
		vmbreak;
	vmcase(lopCdr)
		ctx.valueStack[ctx.sp] = lCdr(ctx.valueStack[ctx.sp]);
		vmbreak;
	vmcase(lopCadr)
		ctx.valueStack[ctx.sp] = lCadr(ctx.valueStack[ctx.sp]);
		vmbreak;
	vmcase(lopList) {
		int len = *ip++;
		lVal cargs = lStackBuildList(ctx.valueStack, ctx.sp, len);
		ctx.sp -= len;
		ctx.valueStack[++ctx.sp] = cargs;
		vmbreak; }
	vmcase(lopClosurePush)
		ctx.valueStack[++ctx.sp] = lValEnvironment(c);
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
		lStoreInClosure(lBytecodeGetOffset16(ip)-1);

		c = lClosureNew(c, closureTry);
		c->exceptionHandler = ctx.valueStack[ctx.sp--];
		ctx.closureStack[++ctx.csp] = c;
		ip+=2;
		vmbreak;
	vmcase(lopThrow) {
		lVal v = ctx.valueStack[ctx.sp];
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
				ctx.valueStack[ctx.sp] = exceptionThrownValue;
				ctx.valueStack[ctx.sp].type = ltException;
				goto topLevelReturn;
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
			lRestoreFromClosure();
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
				lThrow(nv);
			}
			lRestoreFromClosure();
			ctx.sp = c->sp;
			ctx.valueStack[++ctx.sp] = nv;
			break; }
		default:
			lThrow(lValException(lSymTypeError, "Can't apply to following val", fun););
		}
		vmbreak; }
	vmcase(lopMacroDynamic)
	vmcase(lopFnDynamic) {
		const lBytecodeOp curOp = ip[-1];
		lVal cBody = ctx.valueStack[ctx.sp--];
		lVal cDocs = ctx.valueStack[ctx.sp--];
		lVal cArgs = ctx.valueStack[ctx.sp--];
		lVal fun = lLambdaNew(c, cArgs, cBody);
		lClosureSetMeta(fun.vClosure, cDocs);
		if(unlikely(curOp == lopMacroDynamic)){
			fun.type = ltMacro;
		}
		ctx.valueStack[++ctx.sp] = fun;
		vmbreak; }
	vmcase(lopMutableEval)
	vmcase(lopEval) {
		const lBytecodeOp curOp = ip[-1];
		lVal env = ctx.valueStack[ctx.sp--];
		lVal bc = ctx.valueStack[ctx.sp--];
		if(unlikely((env.type != ltEnvironment) || (bc.type != ltBytecodeArr))){
			lThrow(lValException(lSymTypeError, "Can't eval in that", env));
		}

		lStoreInClosure(0);

		if(unlikely(curOp == lopMutableEval)){
			c = ctx.closureStack[++ctx.csp] = env.vClosure;
		} else {
			c = ctx.closureStack[++ctx.csp] = lClosureNew(env.vClosure, closureCall);
		}

		c->text = bc.vBytecodeArr;
		c->ip = c->text->data;
		lRestoreFromClosure();
		lBytecodeEnsureSufficientStack(&ctx);
		lGarbageCollectIfNecessary();

		vmbreak; }
	vmcase(lopApply) {
		const int len = *ip++;
		const int argsSp = ctx.sp;
		ctx.sp -= len;
		lVal fun = ctx.valueStack[ctx.sp--];
		lVal *vs = &ctx.valueStack[argsSp];
		if(unlikely(fun.type == ltKeyword)){
			const lVal self = vs[1-len];
			const lVal nfun = lMethodLookup(fun.vSymbol, self);
			if(unlikely(nfun.type == ltException)){
				lThrow(lValException(lSymTypeError, "Unknown method", lCons(fun, lCons(lValType(&lClassList[self.type]), NIL))));
			}
			fun = nfun;
		}
		switch(fun.type){
		case ltMacro:
		case ltLambda: {
			lStoreInClosure(0);

			ctx.closureStack[++ctx.csp] = c = funCallClosure(fun);
			int vsi = -(len-1);
			for (lVal n = fun.vClosure->args; ; n = n.vList->cdr) {
				if (likely(n.type == ltPair)) {
					if(unlikely(vsi > 0)){
						c->data = lTreeInsert(c->data, n.vList->car.vSymbol, NIL);
					} else {
						c->data = lTreeInsert(c->data, n.vList->car.vSymbol, vs[vsi++]);
					}
					continue;
				} else if(likely(n.type == ltSymbol)) {
					lVal rest = NIL;
					for(int i=0; i >= vsi; i--){
						rest = lCons(vs[i], rest);
					}
					c->data = lTreeInsert(c->data, n.vSymbol, rest);
				}
				break;
			}
			lRestoreFromClosure();
			lBytecodeEnsureSufficientStack(&ctx);
			lGarbageCollectIfNecessary();
			break; }
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
				lThrow(v);
			}
			ctx.valueStack[++ctx.sp] = v;
			break; }
		default:
			lThrow(lValException(lSymTypeError, "Can't apply to following val", fun));
		}
		vmbreak; }
	vmcase(lopApplyCollection) {
		lVal cargs = ctx.valueStack[ctx.sp--];
		lVal fun = ctx.valueStack[ctx.sp--];
		if(unlikely(fun.type == ltKeyword)){
			lVal self = lCar(cargs);
			lVal nfun = lMethodLookup(fun.vSymbol, self);
			if(unlikely(nfun.type == ltException)){
				lThrow(lValException(lSymTypeError, "Unknown method", lCons(fun, lCons(lValType(&lClassList[self.type]), NIL))));
			}
			fun = nfun;
		}
		switch(fun.type){
		case ltMacro:
		case ltLambda:
			lStoreInClosure(0);

			ctx.closureStack[++ctx.csp] = lClosureNewFunCall(cargs, fun);
			c = ctx.closureStack[ctx.csp];

			lRestoreFromClosure();
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
				lThrow(v);
			}
			ctx.valueStack[++ctx.sp] = v;
			break; }
		default:
			lThrow(lValException(lSymTypeError, "Can't apply to following val", fun));
		}
		vmbreak; }
	vmcase(lopRet)
		if(likely(ctx.csp > 0)){
			while(ctx.closureStack[ctx.csp]->type != closureCall){
				if(unlikely(--ctx.csp <= 0)){goto topLevelReturn;}
			}
			lVal ret = ctx.valueStack[ctx.sp];
			c   = ctx.closureStack[--ctx.csp];
			lRestoreFromClosure();
			ctx.text = ops;
			ctx.sp = c->sp;
			ctx.valueStack[++ctx.sp] = ret;
			vmbreak;
		}
	topLevelReturn: {
		lVal ret = ctx.valueStack[ctx.sp];
		free(ctx.closureStack);
		free(ctx.valueStack);
		return ret; }
	}}
}
