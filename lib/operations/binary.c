/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "arithmetic.h"
#include "binary.h"
#include "../casting.h"

static lVal *lnfLogAndI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt &= lCar(vv)->vInt; }
	return t;
}
lVal *lnfLogAnd (lClosure *c, lVal *v){
	lEvalCastIApply(lnfLogAndI,c,v);
}

static lVal *lnfLogIorI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt |= lCar(vv)->vInt; }
	return t;
}
lVal *lnfLogIor (lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lEvalCastIApply(lnfLogIorI,c,v);
}

static lVal *lnfLogXorI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt ^= lCar(vv)->vInt; }
	return t;
}
lVal *lnfLogXor (lClosure *c, lVal *v){
	lEvalCastIApply(lnfLogXorI,c,v);
}

lVal *lnfLogNot (lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lVal *t = lEvalCastSpecific(c,v,ltInt);
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);}
	return lValInt(~lCar(t)->vInt);
}

lVal *lnfAsh(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return lValInt(0);}
	lVal *vals  = lEvalCastSpecific(c,v,ltInt);
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

lVal *lnfAshRight(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return lValInt(0);}
	lVal *vals  = lEvalCastSpecific(c,v,ltInt);
	if(vals == NULL){return lValInt(0);}
	lVal *val   = lCar(vals);
	lVal *shift = lCadr(vals);
	if(shift == NULL){return val;}
	const int sv = shift->vInt;
	if(sv > 0){
		return lValInt(val->vInt >> shift->vInt);
	}else{
		return lValInt(val->vInt << -sv);
	}
}

void lOperationsBinary(lClosure *c){
	lAddInfix(lAddNativeFunc(c,"logand &","[...args]","And ...ARGS together",             lnfLogAnd));
	lAddInfix(lAddNativeFunc(c,"logior |","[...args]","Or ...ARGS",                       lnfLogIor));
	lAddInfix(lAddNativeFunc(c,"logxor ^","[...args]","Xor ...ARGS",                      lnfLogXor));
	lAddNativeFunc(c,"lognot ~","[val]",    "Binary not of VAL",                          lnfLogNot);
	lAddInfix(lAddNativeFunc(c,"ash <<",  "[value amount]","Shift VALUE left AMOUNT bits",lnfAsh));
	lAddInfix(lAddNativeFunc(c,">>",  "[value amount]","Shift VALUE left AMOUNT bits",    lnfAshRight));
}
