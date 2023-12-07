/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

static lVal lValExceptionFloat(lVal v){
	return lValException(lSymTypeError, "This function can only be used with floats",v);
}

static lVal lnfAdd(lVal a, lVal b){
	if(unlikely(a.type == ltNil)){return lValInt(0);}
	if(unlikely(b.type == ltNil)){return a;}
	lType t = lTypecast(a.type, b.type);
	switch(t){
	default:
		return lValExceptionNonNumeric(a);
	case ltInt:
		reqInt(a);
		reqInt(b);
		return lValInt(a.vInt + b.vInt);
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

static lVal lnfSub(lVal a, lVal b){
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
	case ltInt:
		reqInt(a);
		reqInt(b);
		return lValInt(a.vInt - b.vInt);
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

static lVal lnfMul(lVal a, lVal b){
	if(unlikely(a.type == ltNil)){return lValInt(1);}
	if(unlikely(b.type == ltNil)){
		return lValExceptionArity(b, 2);
	}
	lType t = lTypecast(a.type, b.type);
	switch(t){
	default:
		return lValExceptionNonNumeric(a);
	case ltInt:
		reqInt(a);
		reqInt(b);
		return lValInt(a.vInt * b.vInt);
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

static lVal lnfDiv(lVal a, lVal b){
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

static lVal lnfRem(lVal a, lVal b){
	if(unlikely(a.type == ltNil)){return b;}
	if(unlikely(b.type == ltNil)){return a;}
	lType t = lTypecast(a.type, b.type);
	switch(t){
		default:
			return lValExceptionNonNumeric(a);
		case ltInt:
			reqInt(a);
			reqInt(b);
			if(b.vInt == 0){
				return lValException(lSymDivisionByZero, "Module/Dividing by zero is probably not what you wanted", NIL);
			}
			return lValInt(a.vInt % b.vInt);
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

static lVal lnfPow(lVal a, lVal b){
	if(unlikely(b.type == ltNil)){return a;}
	if(unlikely(a.type == ltNil)){
		return lValExceptionArity(b, 2);
	}
	lType t = lTypecast(a.type, b.type);
	switch(t){
	default:
		return lValExceptionFloat(b);
	case ltInt:
		reqInt(a);
		reqInt(b);
		return lValInt(pow(a.vInt, b.vInt));
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

static lVal lnfIncAstI(lVal a){
	if(unlikely(a.type == ltNil)){
		return lValExceptionNonNumeric(a);
	}
	return lValInt(a.vInt + 1);
}

static lVal lnfAddAstI(lVal a, lVal b){
	return lValInt(a.vInt + b.vInt);
}

static lVal lnfSubAstI(lVal a, lVal b){
	return lValInt(a.vInt - b.vInt);
}

static lVal lnfMulAstI(lVal a, lVal b){
	return lValInt(a.vInt * b.vInt);
}

static lVal lnfDivAstI(lVal a, lVal b){
	if(unlikely(b.vInt == 0)){
		return lValException(lSymDivisionByZero, "Can't divide by zero", a);
	}
	return lValInt(a.vInt / b.vInt);
}

static lVal lnfModAstI(lVal a, lVal b){
	if(unlikely(b.vInt == 0)){
		return lValException(lSymDivisionByZero, "Can't divide by zero", a);
	}
	return lValInt(a.vInt % b.vInt);
}

static lVal lnfLogAnd(lVal a, lVal b){
	reqInt(a);
	reqInt(b);
	return lValInt(a.vInt & b.vInt);
}

static lVal lnfLogIor(lVal a, lVal b){
	reqInt(a);
	reqInt(b);
	return lValInt(a.vInt | b.vInt);
}

static lVal lnfLogXor(lVal a, lVal b){
	reqInt(a);
	reqInt(b);
	return lValInt(a.vInt ^ b.vInt);
}

static lVal lnfLogNot(lVal a){
	reqInt(a);
	return lValInt(~a.vInt);
}

static lVal lnfPopCount(lVal a){
	reqInt(a);
#ifdef _MSC_VER
	return lValInt(__popcnt64(a.vInt));
#else
	return lValInt(__builtin_popcountll(a.vInt));
#endif
}

static lVal lnfAsh(lVal a, lVal b){
	reqInt(a);
	reqInt(b);
	const u64 iv = a.vInt;
	const i64 sv = b.vInt;
	return lValInt((sv > 0) ? (iv << sv) : (iv >> -sv));
}

static lVal lnfBitShiftRight(lVal a, lVal b){
	reqInt(a);
	reqInt(b);
	const u64 iv = a.vInt;
	const i64 sv = b.vInt;
	return lValInt((sv > 0) ? (iv >> sv) : (iv << -sv));
}

static lVal lnfAbs(lVal t){
	switch(t.type){
	default:
		return lValExceptionNonNumeric(t);
	case ltFloat:
		return lValFloat(fabs(t.vFloat));
	case ltInt:
		return lValInt(llabs(t.vInt));
	}
}

static lVal lnfCbrt(lVal t){
	switch(t.type){
	default:
		return lValExceptionNonNumeric(t);
	case ltFloat:
		return lValFloat(cbrt(t.vFloat));
	case ltInt:
		return lValFloat(cbrt(t.vInt));
	}
}

static lVal lnfSqrt(lVal t){
	switch(t.type){
	default:
		return lValExceptionNonNumeric(t);
	case ltFloat:
		return lValFloat(sqrt(t.vFloat));
	case ltInt:
		return lValFloat(sqrt(t.vInt));
	}
}

static lVal lnfCeil(lVal t){
	if(likely(t.type == ltFloat)){
		return lValFloat(ceil(t.vFloat));
	}
	return lValExceptionNonNumeric(t);
}

static lVal lnfFloor(lVal t){
	if(likely(t.type == ltFloat)){
		return lValFloat(floor(t.vFloat));
	}
	return lValExceptionNonNumeric(t);
}

static lVal lnfRound(lVal t){
	if(likely(t.type == ltFloat)){
		return lValFloat(round(t.vFloat));
	}
	return lValExceptionNonNumeric(t);
}

static lVal lnfSin(lVal t){
	if(likely(t.type == ltFloat)){
		return lValFloat(sin(t.vFloat));
	}
	return lValExceptionNonNumeric(t);
}

static lVal lnfCos(lVal t){
	if(likely(t.type == ltFloat)){
		return lValFloat(cos(t.vFloat));
	}
	return lValExceptionNonNumeric(t);
}

static lVal lnfTan(lVal t){
	if(likely(t.type == ltFloat)){
		return lValFloat(tan(t.vFloat));
	}
	return lValExceptionNonNumeric(t);
}

static lVal lnfAtanTwo(lVal aA, lVal aB){
	lVal a = requireFloat(aA);
	if(unlikely(a.type == ltException)){
		return a;
	}
	lVal b = requireFloat(aB);
	if(unlikely(b.type == ltException)){
		return b;
	}
	return lValFloat(atan2(a.vFloat, b.vFloat));
}

void lOperationsArithmetic(lClosure *c){
	lAddNativeFuncVV(c,"+",   "(a b)", "Addition",      lnfAdd, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"-",   "(a b)", "Substraction",  lnfSub, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"*",   "(a b)", "Multiplication",lnfMul, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"/",   "(a b)", "Division",      lnfDiv, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"rem", "(a b)", "Remainder",     lnfRem, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"pow", "(a b)", "Return A raised to the power of B",lnfPow, NFUNC_FOLD | NFUNC_PURE);

	lAddNativeFuncVV(c,"add/int", "(a b)", "Return a:int + b:int",  lnfAddAstI, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"sub/int", "(a b)", "Return a:int - b:int",  lnfSubAstI, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"mul/int", "(a b)", "Return a:int * b:int",  lnfMulAstI, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"div/int", "(a b)", "Return a:int / b:int",  lnfDivAstI, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"mod/int", "(a b)", "Return a:int % b:int",  lnfModAstI, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncV (c,"inc/int", "(a)",   "Return a:int + 1",      lnfIncAstI, NFUNC_FOLD | NFUNC_PURE);

	lAddNativeFuncVV(c,"bit-and",  "(a b)", "Bitwise and",          lnfLogAnd, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"bit-or",   "(a b)", "Bitwise or",           lnfLogIor, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncVV(c,"bit-xor",  "(a b)", "Bitwise exclusive or", lnfLogXor, NFUNC_FOLD | NFUNC_PURE);
	lAddNativeFuncV (c,"bit-not",  "(a)",   "Bitwise not",          lnfLogNot, NFUNC_FOLD | NFUNC_PURE);

	lAddNativeFuncVV(c,"bit-shift-left",  "(val amount)", "Shift VALUE left AMOUNT bits",    lnfAsh, NFUNC_PURE);
	lAddNativeFuncVV(c,"bit-shift-right", "(val amount)", "Shift VALUE right AMOUNT bits",    lnfBitShiftRight, NFUNC_PURE);
	lAddNativeFuncV (c,"popcount",        "(val)",        "Return amount of bits set in VAL",lnfPopCount, NFUNC_PURE);

	lAddNativeFuncV (c,"abs",  "(a)",   "Return the absolute value of a", lnfAbs, NFUNC_PURE);
	lAddNativeFuncV (c,"sqrt", "(a)",   "Return the square root of a",    lnfSqrt, NFUNC_PURE);
	lAddNativeFuncV (c,"cbrt", "(a)",   "Return the cube root of a",      lnfCbrt, NFUNC_PURE);
	lAddNativeFuncV (c,"floor","(a)",   "Round a down",                   lnfFloor, NFUNC_PURE);
	lAddNativeFuncV (c,"ceil", "(a)",   "Round a up",                     lnfCeil, NFUNC_PURE);
	lAddNativeFuncV (c,"round","(a)",   "Round a",                        lnfRound, NFUNC_PURE);
	lAddNativeFuncV (c,"sin",  "(a)",   "Sin A",                          lnfSin, NFUNC_PURE);
	lAddNativeFuncV (c,"cos",  "(a)",   "Cos A",                          lnfCos, NFUNC_PURE);
	lAddNativeFuncV (c,"tan",  "(a)",   "Tan A",                          lnfTan, NFUNC_PURE);
	lAddNativeFuncVV(c,"atan2","(y x)", "Arc tangent of y/x",           lnfAtanTwo, NFUNC_PURE);
}
