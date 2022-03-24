/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../exception.h"
#include "../type/native-function.h"
#include "../type/val.h"

static u64 lnfLogAndI(lClosure *c, lVal *l){
        if((l->vList.car == NULL) || (l->vList.car->type != ltInt)){
                lExceptionThrowValClo("type-error","Binary operations can only be done on integers, please convert them beforehand using [int].", l, c);
                return 0;
        }
	u64 acc = l->vList.car->vInt;
	l = l->vList.cdr;
	for(; l; l = l->vList.cdr){
                if((l->vList.car == NULL) || (l->vList.car->type != ltInt)){
                        lExceptionThrowValClo("type-error","Binary operations can only be done on integers, please convert them beforehand using [int].", l, c);
                        return 0;
                }
		acc &= l->vList.car->vInt;
	}
	return acc;
}

static lVal *lnfLogAnd (lClosure *c, lVal *v){
        if(v == NULL){return lValInt(0);}
	return lValInt(lnfLogAndI(c,v));
}

static u64 lnfLogIorI(lClosure *c, lVal *l){
        if((l->vList.car == NULL) || (l->vList.car->type != ltInt)){
                lExceptionThrowValClo("type-error","Binary operations can only be done on integers, please convert them beforehand using [int].", l, c);
                return 0;
        }
	u64 acc = l->vList.car->vInt;
	l = l->vList.cdr;
	for(; l; l = l->vList.cdr){
                if((l->vList.car == NULL) || (l->vList.car->type != ltInt)){
                        lExceptionThrowValClo("type-error","Binary operations can only be done on integers, please convert them beforehand using [int].", l, c);
                        return 0;
                }
		acc |= l->vList.car->vInt;
	}
	return acc;
}

static lVal *lnfLogIor (lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	return lValInt(lnfLogIorI(c,v));
}

static u64 lnfLogXorI(lClosure *c, lVal *l){
        if((l->vList.car == NULL) || (l->vList.car->type != ltInt)){
                lExceptionThrowValClo("type-error","Binary operations can only be done on integers, please convert them beforehand using [int].", l, c);
                return 0;
        }
	u64 acc = l->vList.car->vInt;
	l = l->vList.cdr;
	for(; l; l = l->vList.cdr){
                if((l->vList.car == NULL) || (l->vList.car->type != ltInt)){
                        lExceptionThrowValClo("type-error","Binary operations can only be done on integers, please convert them beforehand using [int].", l, c);
                        return 0;
                }
		acc ^= l->vList.car->vInt;
	}
	return acc;
}

static lVal *lnfLogXor(lClosure *c, lVal *v){
        if(v == NULL){return lValInt(0);}
	return lValInt(lnfLogXorI(c,v));
}

static lVal *lnfLogNot(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(~0);}
	if((v->type != ltPair) || (v->vList.car == NULL) || (v->vList.car->type != ltInt)){
                lExceptionThrowValClo("type-error","Binary operations can only be done on integers, please convert them beforehand using [int].",v, c);
                return NULL;
        }
	return lValInt(~v->vList.car->vInt);
}

#if defined(__TINYC__) || defined(__WATCOMC__)
/* Classic binary divide-and-conquer popcount.
   This is popcount_2() from
   http://en.wikipedia.org/wiki/Hamming_weight */
static inline uint32_t popcount_2(uint32_t x){
    uint32_t m1 = 0x55555555;
    uint32_t m2 = 0x33333333;
    uint32_t m4 = 0x0f0f0f0f;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >>  8;
    return (x + (x >> 16)) & 0x3f;
}
static inline __builtin_popcountll(uint64_t x){
	return popcount_2(x) + popcount_2(x >> 32);
}
#endif

static lVal *lnfPopCount(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	if((v->type != ltPair) || (v->vList.car == NULL) || (v->vList.car->type != ltInt)){
                lExceptionThrowValClo("type-error","Binary operations can only be done on integers, please convert them beforehand using [int].",v, c);
                return NULL;
        }
	return lValInt(__builtin_popcountll(v->vList.car->vInt));
}

static lVal *lnfAsh(lClosure *c, lVal *v){
	lVal *val   = lCar(v);
	lVal *shift = lCadr(v);
        if((val == NULL) || (shift == NULL)){
                lExceptionThrowValClo("arity-error","[ash] needs exactly two arguments", v, c);
                return NULL;
        }
        if(val->type != ltInt){
                lExceptionThrowValClo("type-error","[ash] can only shift integer values", val, c);
                return NULL;
        }
	if(shift->type != ltInt){
                lExceptionThrowValClo("type-error","[ash] can only shift by integer values", shift, c);
                return NULL;
        }
	const int sv = shift->vInt;
	if(sv > 0){
		return lValInt(val->vInt <<  sv);
	}else{
		return lValInt(val->vInt >> -sv);
	}
}

void lOperationsBinary(lClosure *c){
	lAddNativeFunc(c,"logand &","[...args]","And ...ARGS together",             lnfLogAnd);
	lAddNativeFunc(c,"logior |","[...args]","Or ...ARGS",                       lnfLogIor);
	lAddNativeFunc(c,"logxor ^","[...args]","Xor ...ARGS",                      lnfLogXor);
	lAddNativeFunc(c,"lognot ~","[val]",    "Binary not of VAL",                lnfLogNot);
	lAddNativeFunc(c,"ash <<",  "[value amount]","Shift VALUE left AMOUNT bits",lnfAsh);
	lAddNativeFunc(c,"popcount","[val]",    "Return amount of bits set in VAL", lnfPopCount);
}
