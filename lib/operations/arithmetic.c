/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

#include <math.h>
#include <stdlib.h>

static lVal *exceptionThrow(lClosure *c, lVal *v, const char *func){
	(void)func;
	lExceptionThrowValClo("type-error","Can't calculate with non numeric types, please explicitly convert into a numeric form using [int α],[float β],[vec γ].",v, c);
	return NULL;
}

static lVal *exceptionThrowFloat(lClosure *c, lVal *v, const char *func){
	(void)func;
	lExceptionThrowValClo("type-error","This function can only be used with floats, you can use [float α] to explicitly convert into a floating point value",v, c);
	return NULL;
}

lVal *lAdd(lClosure *c, lVal *a, lVal *b){
	if(unlikely(a == NULL)){return lValInt(0);}
	if(unlikely(b == NULL)){return a;}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, a,"addition");
		case ltInt:   return lValInt(requireInt(c,a) + requireInt(c,b));
		case ltFloat: return lValFloat(c, requireFloat(c,a) + requireFloat(c,b));
	}
}

lVal *lSub(lClosure *c, lVal *a, lVal *b){
	if(unlikely(a == NULL)){ throwArityError(c, a, 2); }
	if(unlikely(b == NULL)){
		switch(a->type){
		default:      return exceptionThrow(c, a,"subtraction");
		case ltInt:   return lValInt(-a->vInt);
		case ltFloat: return lValFloat(c, -a->vFloat);
		}
	}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, a,"subtraction");
		case ltInt:   return lValInt(requireInt(c,a) - requireInt(c,b));
		case ltFloat: return lValFloat(c, requireFloat(c,a) - requireFloat(c,b));
	}
}

lVal *lMul(lClosure *c, lVal *a, lVal *b){
	if(unlikely(a == NULL)){return lValInt(1);}
	if(unlikely(b == NULL)){
		throwArityError(c, b, 2);
	}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, a,"multiplication");
		case ltInt:   return lValInt(requireInt(c,a) * requireInt(c,b));
		case ltFloat: return lValFloat(c, requireFloat(c,a) * requireFloat(c,b));
	}
}

lVal *lDiv(lClosure *c, lVal *a, lVal *b){
	if(unlikely((a == NULL) || (b == NULL))){throwArityError(c, b, 2);}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default: return exceptionThrow(c, a,"division");
		case ltInt: {
			const i64 av = requireInt(c,a);
			const i64 bv = requireInt(c,b);
			if(bv == 0){lExceptionThrowValClo("division-by-zero","Dividing by zero is probably not what you wanted", NULL, c);}
			return lValInt(av / bv);}
		case ltFloat: return lValFloat(c,requireFloat(c,a) / requireFloat(c,b));
	}
}

lVal *lRem(lClosure *c, lVal *a, lVal *b){
	if(unlikely(a == NULL)){return b;}
	if(unlikely(b == NULL)){return a;}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, a,"module");
		case ltInt: {
			const i64 av = requireInt(c,a);
			const i64 bv = requireInt(c,b);
			if(bv == 0){lExceptionThrowValClo("division-by-zero","Module/Dividing by zero is probably not what you wanted", NULL, c);}
			return lValInt(av % bv);}
		case ltFloat: return lValFloat(c, fmod(requireFloat(c,a), requireFloat(c,b)));
	}
}

static lVal *lnfAdd(lClosure *c, lVal *v){
	return lAdd(c, lCar(v), lCadr(v));
}

static lVal *lnfSub(lClosure *c, lVal *v){
	return lSub(c, lCar(v), lCadr(v));
}

static lVal *lnfMul(lClosure *c, lVal *v){
	return lMul(c, lCar(v), lCadr(v));
}

static lVal *lnfDiv(lClosure *c, lVal *v){
	return lDiv(c, lCar(v), lCadr(v));
}

static lVal *lnfRem(lClosure *c, lVal *v){
	return lRem(c, lCar(v), lCadr(v));
}

static lVal *lnfPow(lClosure *c, lVal *v){
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(unlikely(b == NULL)){return a;}
	if(unlikely(a == NULL)){
		throwArityError(c, v, 2);
	}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrowFloat(c, v,"power");
		case ltInt:   return lValInt(pow(requireInt(c,a),  requireInt(c,b)));
		case ltFloat: return lValFloat(c, pow(requireFloat(c,a), requireFloat(c,b)));
	}
}

static lVal *lnfIncAstI(lClosure *c, lVal *v){
	if(unlikely(v == NULL) || unlikely(v->vList.car == NULL)){
		return exceptionThrow(c, v, "inc/int");
	}
	const i64 a = v->vList.car->vInt;
	return lValInt(a + 1);
}

static lVal *lnfAddAstI(lClosure *c, lVal *v){
	(void)c;
	const i64 a = v->vList.car->vInt;
	const i64 b = v->vList.cdr->vList.car->vInt;
	return lValInt(a + b);
}

static lVal *lnfSubAstI(lClosure *c, lVal *v){
	(void)c;
	const i64 a = v->vList.car->vInt;
	const i64 b = v->vList.cdr->vList.car->vInt;
	return lValInt(a - b);
}

static lVal *lnfMulAstI(lClosure *c, lVal *v){
	(void)c;
	const i64 a = v->vList.car->vInt;
	const i64 b = v->vList.cdr->vList.car->vInt;
	return lValInt(a * b);
}

static lVal *lnfDivAstI(lClosure *c, lVal *v){
	(void)c;
	const i64 a = v->vList.car->vInt;
	const i64 b = v->vList.cdr->vList.car->vInt;
	return lValInt(a / b);
}

static lVal *lnfModAstI(lClosure *c, lVal *v){
	(void)c;
	const i64 a = v->vList.car->vInt;
	const i64 b = v->vList.cdr->vList.car->vInt;
	return lValInt(a % b);
}

static lVal *lnfPowAstI(lClosure *c, lVal *v){
	(void)c;
	const i64 a = v->vList.car->vInt;
	const i64 b = v->vList.cdr->vList.car->vInt;
	return lValInt(pow(a,b));
}

static lVal *lnfLogAnd(lClosure *c, lVal *v){
	return lValInt(requireInt(c, lCar(v)) & requireInt(c, lCadr(v)));
}

static lVal *lnfLogIor(lClosure *c, lVal *v){
	return lValInt(requireInt(c, lCar(v)) | requireInt(c, lCadr(v)));
}

static lVal *lnfLogXor(lClosure *c, lVal *v){
        return lValInt(requireInt(c, lCar(v)) ^ requireInt(c, lCadr(v)));
}

static lVal *lnfLogNot(lClosure *c, lVal *v){
	return lValInt(~requireInt(c, lCar(v)));
}

static lVal *lnfPopCount(lClosure *c, lVal *v){
	const i64 iv = requireInt(c, lCar(v));
#ifdef _MSC_VER
	return lValInt(__popcnt64(iv));
#else
	return lValInt(__builtin_popcountll(iv));
#endif
}

static lVal *lnfAsh(lClosure *c, lVal *v){
	const u64 iv = requireInt(c, lCar(v));
	const i64 sv = requireInt(c, lCadr(v));
	return lValInt((sv > 0) ? (iv <<  sv) : (iv >> -sv));
}

lVal *lnfAbs(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrow(c, v,"absolute");
		case ltFloat: return lValFloat(c,fabs(t->vFloat));
		case ltInt:   return lValInt(llabs(t->vInt));
	}
}

lVal *lnfCbrt(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrow(c, v,"squareroot");
		case ltFloat: return lValFloat(c, cbrt(t->vFloat));
		case ltInt:   return lValFloat(c, cbrt(t->vInt));
	}
}

lVal *lnfSqrt(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrow(c, v,"squareroot");
		case ltFloat: return lValFloat(c, sqrt(t->vFloat));
		case ltInt:   return lValFloat(c, sqrt(t->vInt));
	}
}

lVal *lnfCeil(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrow(c, v,"ceil");
		case ltFloat: return lValFloat(c, ceil(t->vFloat));
	}
}

lVal *lnfFloor(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrow(c, v,"floor");
		case ltFloat: return lValFloat(c, floor(t->vFloat));
	}
}

lVal *lnfRound(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrow(c, v,"round");
		case ltFloat: return lValFloat(c, round(t->vFloat));
	}
}

lVal *lnfSin(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrowFloat(c, v,"sin");
		case ltFloat: return lValFloat(c, sin(t->vFloat));
	}
}

lVal *lnfCos(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrowFloat(c, v,"cos");
		case ltFloat: return lValFloat(c, cos(t->vFloat));
	}
}

lVal *lnfTan(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	typeswitch(t){
		default:      return exceptionThrowFloat(c, v,"tan");
		case ltFloat: return lValFloat(c, tan(t->vFloat));
	}
}

lVal *lnfAtanTwo(lClosure *c, lVal *v){
	const double y = requireFloat(c,  lCar(v));
	const double x = requireFloat(c, lCadr(v));
	return lValFloat(c, atan2(y, x));
}

void lOperationsArithmetic(lClosure *c){
	lAddNativeFuncPureFold(c,"+",   "[a b]", "Addition",      lnfAdd);
	lAddNativeFuncPureFold(c,"-",   "[a b]", "Substraction",  lnfSub);
	lAddNativeFuncPureFold(c,"*",   "[a b]", "Multiplication",lnfMul);
	lAddNativeFuncPureFold(c,"/",   "[a b]", "Division",      lnfDiv);
	lAddNativeFuncPureFold(c,"rem", "[a b]", "Remainder",     lnfRem);
	lAddNativeFuncPureFold(c,"pow", "[a b]", "Return A raised to the power of B",lnfPow);

	lAddNativeFuncPureFold(c,"add/int", "[a b]", "Return a:int + b:int",  lnfAddAstI);
	lAddNativeFuncPureFold(c,"sub/int", "[a b]", "Return a:int - b:int",  lnfSubAstI);
	lAddNativeFuncPureFold(c,"mul/int", "[a b]", "Return a:int * b:int",  lnfMulAstI);
	lAddNativeFuncPureFold(c,"div/int", "[a b]", "Return a:int / b:int",  lnfDivAstI);
	lAddNativeFuncPureFold(c,"mod/int", "[a b]", "Return a:int % b:int",  lnfModAstI);
	lAddNativeFuncPureFold(c,"pow/int", "[a b]", "Return a:int ** b:int", lnfPowAstI);
	lAddNativeFuncPureFold(c,"inc/int", "[a]",   "Return a:int + 1",      lnfIncAstI);

	lAddNativeFuncPureFold(c,"bit-and",  "[a b]", "Bitwise and",          lnfLogAnd);
	lAddNativeFuncPureFold(c,"bit-or",   "[a b]", "Bitwise or",           lnfLogIor);
	lAddNativeFuncPureFold(c,"bit-xor",  "[a b]", "Bitwise exclusive or", lnfLogXor);
	lAddNativeFuncPureFold(c,"bit-not",  "[a]",   "Bitwise not",          lnfLogNot);

	lAddNativeFuncPure(c,"bit-shift-left", "[val amount]","Shift VALUE left AMOUNT bits",    lnfAsh);
	lAddNativeFuncPure(c,"popcount",       "[val]",       "Return amount of bits set in VAL",lnfPopCount);

	lAddNativeFuncPure(c,"abs",  "[a]", "Return the absolute value of a", lnfAbs);
	lAddNativeFuncPure(c,"sqrt", "[a]", "Return the square root of a",    lnfSqrt);
	lAddNativeFuncPure(c,"cbrt", "[a]", "Return the cube root of a",      lnfCbrt);
	lAddNativeFuncPure(c,"floor","[a]", "Round a down",                   lnfFloor);
	lAddNativeFuncPure(c,"ceil", "[a]", "Round a up",                     lnfCeil);
	lAddNativeFuncPure(c,"round","[a]", "Round a",                        lnfRound);
	lAddNativeFuncPure(c,"sin",  "[a]", "Sin A",                          lnfSin);
	lAddNativeFuncPure(c,"cos",  "[a]", "Cos A",                          lnfCos);
	lAddNativeFuncPure(c,"tan",  "[a]", "Tan A",                          lnfTan);
	lAddNativeFuncPure(c,"atan2","[y x]", "Arc tangent of y/x",           lnfAtanTwo);
}
