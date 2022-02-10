/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"

#include "../type-system.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/val.h"

static int lnfLogAndI(const lVal *l){
	int acc = l->vList.car->vInt;
	l = l->vList.cdr;
	for(; l; l = l->vList.cdr){
		acc &= l->vList.car->vInt;
	}
	return acc;
}

static lVal *lnfLogAnd (lClosure *c, lVal *v){
	lVal *t = lCast(c,v,ltInt);
	if((t == NULL) || (t->vList.car == NULL) || (t->vList.car->type != ltInt)){return lValInt(0);}
	lRootsValPush(t);
	return lValInt(lnfLogAndI(t));
}

static int lnfLogIorI(const lVal *l){
	int acc = l->vList.car->vInt;
	l = l->vList.cdr;
	for(; l; l = l->vList.cdr){
		acc |= l->vList.car->vInt;
	}
	return acc;
}

static lVal *lnfLogIor (lClosure *c, lVal *v){
	lVal *t = lCast(c,v,ltInt);
	if((t == NULL) || (t->vList.car == NULL) || (t->vList.car->type != ltInt)){return lValInt(0);}
	lRootsValPush(t);
	return lValInt(lnfLogIorI(t));
}

static int lnfLogXorI(const lVal *l){
	int acc = l->vList.car->vInt;
	l = l->vList.cdr;
	for(; l; l = l->vList.cdr){
		acc ^= l->vList.car->vInt;
	}
	return acc;
}

static lVal *lnfLogXor(lClosure *c, lVal *v){
	lVal *t = lCast(c,v,ltInt);
	if((t == NULL) || (t->vList.car == NULL) || (t->vList.car->type != ltInt)){return lValInt(0);}
	lRootsValPush(t);
	return lValInt(lnfLogXorI(t));
}

static lVal *lnfLogNot(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lVal *t = lCast(c,v,ltInt);
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);}
	return lValInt(~lCar(t)->vInt);
}

#ifdef __TINYC__
static inline uint64_t __builtin_popcount(uint64_t x){
    uint64_t m1 = 0x5555555555555555;
    uint64_t m2 = 0x3333333333333333;
    uint64_t m4 = 0x0f0f0f0f0f0f0f0f;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >>  8;
    return (x + (x >> 16)) & 0x3f;
}
#endif

static lVal *lnfPopCount(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lVal *t = lCast(c,v,ltInt);
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);}
	return lValInt(__builtin_popcount(lCar(t)->vInt));
}

static lVal *lnfAsh(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return lValInt(0);}
	lVal *vals  = lCast(c,v,ltInt);
	if(vals == NULL){return lValInt(0);}
	lVal *val   = lCar(vals);
	lVal *shift = lCadr(vals);
	if(shift == NULL){return val;}
	const int sv = shift->vInt;
	if(sv > 0){
		return lValInt(val->vInt << shift->vInt);
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
