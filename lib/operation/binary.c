/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../exception.h"
#include "../type/native-function.h"
#include "../type/val.h"
#include "../misc/popcount.h"

static lVal *lnfLogAnd(lClosure *c, lVal *v){
	u64 acc = requireInt(c, lCar(v));
	for(lVal *l = lCdr(v); l && l->type == ltPair; l = l->vList.cdr){
		acc &= requireInt(c, lCar(l));
	}
	return lValInt(acc);
}

static lVal *lnfLogIor(lClosure *c, lVal *v){
	u64 acc = requireInt(c, lCar(v));
	for(lVal *l = lCdr(v); l && l->type == ltPair; l = l->vList.cdr){
		acc |= requireInt(c, lCar(l));
	}
	return lValInt(acc);
}

static lVal *lnfLogXor(lClosure *c, lVal *v){
        u64 acc = requireInt(c, lCar(v));
	for(lVal *l = lCdr(v); l && l->type == ltPair; l = l->vList.cdr){
		acc ^= requireInt(c, lCar(l));
	}
	return lValInt(acc);
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

void lOperationsBinary(lClosure *c){
	lAddNativeFunc(c,"logand",  "args",        "And ARGS together",               lnfLogAnd);
	lAddNativeFunc(c,"logior",  "args",        "Or ARGS",                         lnfLogIor);
	lAddNativeFunc(c,"logxor",  "args",        "Xor ARGS",                        lnfLogXor);
	lAddNativeFunc(c,"lognot",  "[val]",       "Binary not of VAL",               lnfLogNot);
	lAddNativeFunc(c,"ash",     "[val amount]","Shift VALUE left AMOUNT bits",    lnfAsh);
	lAddNativeFunc(c,"popcount","[val]",       "Return amount of bits set in VAL",lnfPopCount);
}
