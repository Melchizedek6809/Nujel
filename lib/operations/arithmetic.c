/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../nujel-private.h"

#include <math.h>

#ifdef __WATCOMC__
#define fmodf(X,Y) fmod(X,Y)
#endif

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
		case ltFloat: return lValFloat(requireFloat(c,a) + requireFloat(c,b));
		case ltVec:   return lValVec(vecAdd(requireVecCompatible(c,a), requireVecCompatible(c,b)));
	}
}

static lVal *lnfAdd(lClosure *c, lVal *v){
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(lCddr(v)){
		return lnfAdd(c, lCons(lAdd(c, a, b), lCddr(v)));
	} else {
		return lAdd(c, a, b);
	}
}

lVal *lSub(lClosure *c, lVal *a, lVal *b){
	if(unlikely(a == NULL)){ throwArityError(c, a, 2); }
	if(unlikely(b == NULL)){
		switch(a->type){
		default:      return exceptionThrow(c, a,"subtraction");
		case ltInt:   return lValInt(-a->vInt);
		case ltFloat: return lValFloat(-a->vFloat);
		case ltVec:   return lValVec(vecInvert(a->vVec));
		}
	}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, a,"subtraction");
		case ltInt:   return lValInt(requireInt(c,a) - requireInt(c,b));
		case ltFloat: return lValFloat(requireFloat(c,a) - requireFloat(c,b));
		case ltVec:   return lValVec(vecSub(requireVecCompatible(c,a), requireVecCompatible(c,b)));
	}
}

static lVal *lnfSub(lClosure *c, lVal *v){
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(lCddr(v)){
		return lnfSub(c, lCons(lSub(c, a, b), lCddr(v)));
	} else {
		return lSub(c, a, b);
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
		case ltFloat: return lValFloat(requireFloat(c,a) * requireFloat(c,b));
		case ltVec:   return lValVec(vecMul(requireVecCompatible(c,a), requireVecCompatible(c,b)));\
	}
}

static lVal *lnfMul(lClosure *c, lVal *v){
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(lCddr(v)){
		return lnfMul(c, lCons(lMul(c, a, b), lCddr(v)));
	} else {
		return lMul(c, a, b);
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
		case ltFloat: return lValFloat(requireFloat(c,a) / requireFloat(c,b));
		case ltVec:   return lValVec(vecDiv(requireVecCompatible(c,a), requireVecCompatible(c,b)));
	}
}

static lVal *lnfDiv(lClosure *c, lVal *v){
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(lCddr(v)){
		return lnfDiv(c, lCons(lDiv(c, a, b), lCddr(v)));
	} else {
		return lDiv(c, a, b);
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
		case ltFloat: return lValFloat(fmod(requireFloat(c,a), requireFloat(c,b)));
		case ltVec:   return lValVec(vecMod(requireVecCompatible(c,a), requireVecCompatible(c,b)));
	}
}

static lVal *lnfRem(lClosure *c, lVal *v){
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(lCddr(v)){
		return lnfRem(c, lCons(lRem(c, a, b), lCddr(v)));
	} else {
		return lRem(c, a, b);
	}
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
		case ltFloat: return lValFloat(pow(requireFloat(c,a), requireFloat(c,b)));
		case ltVec:   return lValVec(vecPow(requireVecCompatible(c,a), requireVecCompatible(c,b)));
	}
}


static lVal *lnfAddAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a + b);
}

static lVal *lnfSubAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a - b);
}

static lVal *lnfMulAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a * b);
}

static lVal *lnfDivAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a / b);
}

static lVal *lnfModAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
	return lValInt(a % b);
}

static lVal *lnfPowAstI(lClosure *c, lVal *v){
	(void)c;
	const int a = v->vList.car->vInt;
	const int b = v->vList.cdr->vList.car->vInt;
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
	return lValInt(__builtin_popcountll(requireInt(c, lCar(v))));
}

static lVal *lnfAsh(lClosure *c, lVal *v){
	const i64 iv = requireInt(c, lCar(v));
	const i64 sv = requireInt(c, lCadr(v));
	return lValInt((sv > 0) ? (iv <<  sv) : (iv >> -sv));
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

	lAddNativeFuncPureFold(c,"bit-and",  "[a b]", "Bitwise and",          lnfLogAnd);
	lAddNativeFuncPureFold(c,"bit-or",   "[a b]", "Bitwise or",           lnfLogIor);
	lAddNativeFuncPureFold(c,"bit-xor",  "[a b]", "Bitwise exclusive or", lnfLogXor);
	lAddNativeFuncPureFold(c,"bit-not",  "[a]",   "Bitwise not",          lnfLogNot);

	lAddNativeFuncPure(c,"bit-shift-left",     "[val amount]","Shift VALUE left AMOUNT bits",    lnfAsh);
	lAddNativeFuncPure(c,"popcount","[val]",       "Return amount of bits set in VAL",lnfPopCount);
}
