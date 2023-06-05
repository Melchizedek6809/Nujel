/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <math.h>
#include <stdlib.h>

static lVal lAdd(lVal a, lVal b){
	if(unlikely(a.type == ltNil)){return lValInt(0);}
	if(unlikely(b.type == ltNil)){return a;}
	lType t = lTypecast(a.type, b.type);
	switch(t){
	default:
		return lValExceptionNonNumeric(a);
	case ltInt: {
		lVal av = requireInt(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireInt(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValInt(av.vInt + bv.vInt); }
	case ltFloat: {
		lVal av = requireFloat(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireFloat(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValFloat(av.vFloat + bv.vFloat); }
	}
}

static lVal lSub(lVal a, lVal b){
	if(unlikely(a.type == ltNil)){
		return lValExceptionArity(a, 2);
	}
	if(unlikely(b.type == ltNil)){
		switch(a.type){
		default:
			return lValExceptionNonNumeric(a);
		case ltInt:
			return lValInt(-a.vInt);
		case ltFloat:
			return lValFloat(-a.vFloat);
		}
	}
	lType t = lTypecast(a.type, b.type);
	switch(t){
	default:
		return lValExceptionNonNumeric(a);
	case ltInt: {
		lVal av = requireInt(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireInt(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValInt(av.vInt - bv.vInt); }
	case ltFloat: {
		lVal av = requireFloat(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireFloat(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValFloat(av.vFloat - bv.vFloat); }
	}
}

static lVal lMul(lVal a, lVal b){
	if(unlikely(a.type == ltNil)){return lValInt(1);}
	if(unlikely(b.type == ltNil)){
		return lValExceptionArity(b, 2);
	}
	lType t = lTypecast(a.type, b.type);
	switch(t){
	default:
		return lValExceptionNonNumeric(a);
	case ltInt: {
		lVal av = requireInt(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireInt(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValInt(av.vInt * bv.vInt); }
	case ltFloat: {
		lVal av = requireFloat(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireFloat(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValFloat(av.vFloat * bv.vFloat); }
	}
}

static lVal lDiv(lVal a, lVal b){
	if(unlikely((a.type == ltNil) || (b.type == ltNil))){
		return lValExceptionArity(b, 2);
	}
	lType t = lTypecast(a.type, b.type);
	switch(t){
	default:
		return lValExceptionNonNumeric(a);
	case ltInt:
	case ltFloat: {
		lVal av = requireFloat(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireFloat(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValFloat(av.vFloat / bv.vFloat); }
	}
}

static lVal lRem(lVal a, lVal b){
	if(unlikely(a.type == ltNil)){return b;}
	if(unlikely(b.type == ltNil)){return a;}
	lType t = lTypecast(a.type, b.type);
	switch(t){
		default:
			return lValExceptionNonNumeric(a);
		case ltInt: {
			const lVal av = requireInt(a);
			if(unlikely(av.type == ltException)){
				return av;
			}
			const lVal bv = requireInt(b);
			if(unlikely(bv.type == ltException)){
				return bv;
			}
			if(bv.vInt == 0){
				return lValException("division-by-zero","Module/Dividing by zero is probably not what you wanted", NIL);
			}
			return lValInt(av.vInt % bv.vInt);}
		case ltFloat: {
			lVal av = requireFloat(a);
			if(unlikely(av.type == ltException)){
				return av;
			}
			lVal bv = requireFloat(b);
			if(unlikely(bv.type == ltException)){
				return bv;
			}
			return lValFloat(fmod(av.vFloat, bv.vFloat)); }
	}
}

static lVal lnfAdd(lClosure *c, lVal v){
	(void)c;
	return lAdd(lCar(v), lCadr(v));
}

static lVal lnfSub(lClosure *c, lVal v){
	(void)c;
	return lSub(lCar(v), lCadr(v));
}

static lVal lnfMul(lClosure *c, lVal v){
	(void)c;
	return lMul(lCar(v), lCadr(v));
}

static lVal lnfDiv(lClosure *c, lVal v){
	(void)c;
	return lDiv(lCar(v), lCadr(v));
}

static lVal lnfRem(lClosure *c, lVal v){
	(void)c;
	return lRem(lCar(v), lCadr(v));
}

static lVal lnfPow(lClosure *c, lVal v){
	(void)c;
	lVal a = lCar(v);
	lVal b = lCadr(v);
	if(unlikely(b.type == ltNil)){return a;}
	if(unlikely(a.type == ltNil)){
		return lValExceptionArity(v, 2);
	}
	lType t = lTypecast(a.type, b.type);
	switch(t){
	default:
		return lValExceptionFloat(v);
	case ltInt: {
		lVal av = requireInt(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireInt(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValInt(pow(av.vInt,  bv.vInt)); }
	case ltFloat: {
		lVal av = requireFloat(a);
		if(unlikely(av.type == ltException)){
			return av;
		}
		lVal bv = requireFloat(b);
		if(unlikely(bv.type == ltException)){
			return bv;
		}
		return lValFloat(pow(av.vFloat, bv.vFloat)); }
	}
}

static lVal lnfIncAstI(lClosure *c, lVal v){
	(void)c;
	if(unlikely(v.type == ltNil) || unlikely(v.vList->car.type == ltNil)){
		return lValExceptionNonNumeric(v);
	}
	const i64 a = v.vList->car.vInt;
	return lValInt(a + 1);
}

static lVal lnfAddAstI(lClosure *c, lVal v){
	(void)c;
	const i64 a = v.vList->car.vInt;
	const i64 b = v.vList->cdr.vList->car.vInt;
	return lValInt(a + b);
}

static lVal lnfSubAstI(lClosure *c, lVal v){
	(void)c;
	const i64 a = v.vList->car.vInt;
	const i64 b = v.vList->cdr.vList->car.vInt;
	return lValInt(a - b);
}

static lVal lnfMulAstI(lClosure *c, lVal v){
	(void)c;
	const i64 a = v.vList->car.vInt;
	const i64 b = v.vList->cdr.vList->car.vInt;
	return lValInt(a * b);
}

static lVal lnfDivAstI(lClosure *c, lVal v){
	(void)c;
	if(unlikely(((v.type == ltNil) || (v.vList->car.type == ltNil) || (v.vList->cdr.type == ltNil) || (v.vList->cdr.vList->car.type == ltNil)))){
		return lValException("arity-error", "Expected 2 arguments", v);
	}
	const i64 a = v.vList->car.vInt;
	const i64 b = v.vList->cdr.vList->car.vInt;
	if(unlikely(b == 0)){
		return lValException("divide-by-zero", "Can't divide by zero", v);
	}
	return lValInt(a / b);
}

static lVal lnfModAstI(lClosure *c, lVal v){
	(void)c;
	const i64 a = v.vList->car.vInt;
	const i64 b = v.vList->cdr.vList->car.vInt;
	return lValInt(a % b);
}

static lVal lnfPowAstI(lClosure *c, lVal v){
	(void)c;
	const i64 a = v.vList->car.vInt;
	const i64 b = v.vList->cdr.vList->car.vInt;
	return lValInt(pow(a,b));
}

static lVal lnfLogAnd(lClosure *c, lVal v){
	(void)c;
	lVal av = requireInt(lCar(v));
	if(unlikely(av.type == ltException)){
		return av;
	}
	lVal bv = requireInt(lCadr(v));
	if(unlikely(bv.type == ltException)){
		return bv;
	}
	return lValInt(av.vInt & bv.vInt);
}

static lVal lnfLogIor(lClosure *c, lVal v){
	(void)c;
	lVal av = requireInt(lCar(v));
	if(unlikely(av.type == ltException)){
		return av;
	}
	lVal bv = requireInt(lCadr(v));
	if(unlikely(bv.type == ltException)){
		return bv;
	}
	return lValInt(av.vInt | bv.vInt);
}

static lVal lnfLogXor(lClosure *c, lVal v){
        (void)c;
	lVal av = requireInt(lCar(v));
	if(unlikely(av.type == ltException)){
		return av;
	}
	lVal bv = requireInt(lCadr(v));
	if(unlikely(bv.type == ltException)){
		return bv;
	}
	return lValInt(av.vInt ^ bv.vInt);
}

static lVal lnfLogNot(lClosure *c, lVal v){
	(void)c;
	lVal av = requireInt(lCar(v));
	if(unlikely(av.type == ltException)){
		return av;
	}
	return lValInt(~av.vInt);
}

static lVal lnfPopCount(lClosure *c, lVal v){
	(void)c;
	lVal car = requireInt(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	const i64 iv = car.vInt;
#ifdef _MSC_VER
	return lValInt(__popcnt64(iv));
#else
	return lValInt(__builtin_popcountll(iv));
#endif
}

static lVal lnfAsh(lClosure *c, lVal v){
	(void)c;
	lVal av = requireInt(lCar(v));
	if(unlikely(av.type == ltException)){
		return av;
	}
	lVal bv = requireInt(lCadr(v));
	if(unlikely(bv.type == ltException)){
		return bv;
	}
	const u64 iv = av.vInt;
	const i64 sv = bv.vInt;
	return lValInt((sv > 0) ? (iv <<  sv) : (iv >> -sv));
}

lVal lnfAbs(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	switch(t.type){
	default:
		return lValExceptionNonNumeric(v);
	case ltFloat:
		return lValFloat(fabs(t.vFloat));
	case ltInt:
		return lValInt(llabs(t.vInt));
	}
}

lVal lnfCbrt(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	switch(t.type){
	default:
		return lValExceptionNonNumeric(v);
	case ltFloat:
		return lValFloat(cbrt(t.vFloat));
	case ltInt:
		return lValFloat(cbrt(t.vInt));
	}
}

lVal lnfSqrt(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	switch(t.type){
	default:
		return lValExceptionNonNumeric(v);
	case ltFloat:
		return lValFloat(sqrt(t.vFloat));
	case ltInt:
		return lValFloat(sqrt(t.vInt));
	}
}

lVal lnfCeil(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	if(likely(t.type == ltFloat)){
		return lValFloat(ceil(t.vFloat));
	}
	return lValExceptionNonNumeric(v);
}

lVal lnfFloor(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	if(likely(t.type == ltFloat)){
		return lValFloat(floor(t.vFloat));
	}
	return lValExceptionNonNumeric(v);
}

lVal lnfRound(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	if(likely(t.type == ltFloat)){
		return lValFloat(round(t.vFloat));
	}
	return lValExceptionNonNumeric(v);
}

lVal lnfSin(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	if(likely(t.type == ltFloat)){
		return lValFloat(sin(t.vFloat));
	}
	return lValExceptionNonNumeric(v);
}

lVal lnfCos(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	if(likely(t.type == ltFloat)){
		return lValFloat(cos(t.vFloat));
	}
	return lValExceptionNonNumeric(v);
}

lVal lnfTan(lClosure *c, lVal v){
	(void)c;
	lVal t = lCar(v);
	if(likely(t.type == ltFloat)){
		return lValFloat(tan(t.vFloat));
	}
	return lValExceptionNonNumeric(v);
}

lVal lnfAtanTwo(lClosure *c, lVal v){
	(void)c;
	lVal a = requireFloat(lCar(v));
	if(unlikely(a.type == ltException)){
		return a;
	}
	lVal b = requireFloat(lCadr(v));
	if(unlikely(b.type == ltException)){
		return b;
	}
	return lValFloat(atan2(a.vFloat, b.vFloat));
}

void lOperationsArithmetic(lClosure *c){
	lAddNativeFuncPureFold(c,"+",   "(a b)", "Addition",      lnfAdd);
	lAddNativeFuncPureFold(c,"-",   "(a b)", "Substraction",  lnfSub);
	lAddNativeFuncPureFold(c,"*",   "(a b)", "Multiplication",lnfMul);
	lAddNativeFuncPureFold(c,"/",   "(a b)", "Division",      lnfDiv);
	lAddNativeFuncPureFold(c,"rem", "(a b)", "Remainder",     lnfRem);
	lAddNativeFuncPureFold(c,"pow", "(a b)", "Return A raised to the power of B",lnfPow);

	lAddNativeFuncPureFold(c,"add/int", "(a b)", "Return a:int + b:int",  lnfAddAstI);
	lAddNativeFuncPureFold(c,"sub/int", "(a b)", "Return a:int - b:int",  lnfSubAstI);
	lAddNativeFuncPureFold(c,"mul/int", "(a b)", "Return a:int * b:int",  lnfMulAstI);
	lAddNativeFuncPureFold(c,"div/int", "(a b)", "Return a:int / b:int",  lnfDivAstI);
	lAddNativeFuncPureFold(c,"mod/int", "(a b)", "Return a:int % b:int",  lnfModAstI);
	lAddNativeFuncPureFold(c,"pow/int", "(a b)", "Return a:int ** b:int", lnfPowAstI);
	lAddNativeFuncPureFold(c,"inc/int", "(a)",   "Return a:int + 1",      lnfIncAstI);

	lAddNativeFuncPureFold(c,"bit-and",  "(a b)", "Bitwise and",          lnfLogAnd);
	lAddNativeFuncPureFold(c,"bit-or",   "(a b)", "Bitwise or",           lnfLogIor);
	lAddNativeFuncPureFold(c,"bit-xor",  "(a b)", "Bitwise exclusive or", lnfLogXor);
	lAddNativeFuncPureFold(c,"bit-not",  "(a)",   "Bitwise not",          lnfLogNot);

	lAddNativeFuncPure(c,"bit-shift-left", "(val amount)","Shift VALUE left AMOUNT bits",    lnfAsh);
	lAddNativeFuncPure(c,"popcount",       "(val)",       "Return amount of bits set in VAL",lnfPopCount);

	lAddNativeFuncPure(c,"abs",  "(a)", "Return the absolute value of a", lnfAbs);
	lAddNativeFuncPure(c,"sqrt", "(a)", "Return the square root of a",    lnfSqrt);
	lAddNativeFuncPure(c,"cbrt", "(a)", "Return the cube root of a",      lnfCbrt);
	lAddNativeFuncPure(c,"floor","(a)", "Round a down",                   lnfFloor);
	lAddNativeFuncPure(c,"ceil", "(a)", "Round a up",                     lnfCeil);
	lAddNativeFuncPure(c,"round","(a)", "Round a",                        lnfRound);
	lAddNativeFuncPure(c,"sin",  "(a)", "Sin A",                          lnfSin);
	lAddNativeFuncPure(c,"cos",  "(a)", "Cos A",                          lnfCos);
	lAddNativeFuncPure(c,"tan",  "(a)", "Tan A",                          lnfTan);
	lAddNativeFuncPure(c,"atan2","(y x)", "Arc tangent of y/x",           lnfAtanTwo);
}
