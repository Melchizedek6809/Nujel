/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "conditional.h"
#include "../casting.h"

static lVal *lnfNot(lClosure *c, lVal *v){
	lVal *a = lnfBool(c,v);
	return lValBool(a == NULL ? true : !a->vBool);
}

static lVal *lnfAnd(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(true);}
	lVal *t = lnfBool(c,v);
	if((t == NULL) || (!t->vBool)){return lValBool(false);}
	return lnfAnd(c,lCdr(v));
}

static lVal *lnfOr(lClosure *c, lVal *v){
	if(v == NULL){return lValBool(false);}
	lVal *t = lnfBool(c,v);
	if((t != NULL) && t->vBool){return lValBool(true);}
	return lnfOr(c,lCdr(v));
}

static lVal *lnfCond(lClosure *c, lVal *v){
	if(v == NULL)        {return NULL;}
	if(v->type != ltPair){return NULL;}
	lVal *t = lCar(v);
	lVal *b = lnfBool(c,lCar(t));
	if((b != NULL) && b->vBool){
		return lLastCar(lApply(c,lCdr(t),lEval));
	}
	return lnfCond(c,lCdr(v));
}

static lVal *lnfWhen(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *condition = lnfBool(c,lEval(c,lCar(v)));
	if((condition == NULL) || (condition->type != ltBool) || (!condition->vBool)){return NULL;}
	return lnfBegin(c,lCdr(v));
}

static lVal *lnfUnless(lClosure *c, lVal *v){
	if(v == NULL){return NULL;}
	lVal *condition = lnfBool(c,lEval(c,lCar(v)));
	if((condition != NULL) && (condition->type == ltBool) && (condition->vBool)){return NULL;}
	return lnfBegin(c,lCdr(v));
}

static lVal *lnfIf(lClosure *c, lVal *v){
	if(v == NULL)         {return NULL;}
	if(v->type != ltPair) {return NULL;}
	lVal *pred = lnfBool(c,lCar(v));
	v = lCdr(v);
	if(v == NULL)         {return NULL;}
	if(((pred == NULL) || (pred->vBool == false)) && (lCdr(v) != NULL)){v = lCdr(v);}
	return lEval(c,lCar(v));
}

void lOperationsConditional(lClosure *c){
	lAddNativeFunc(c,"if",             "[pred? then ...else]","Evalute then if pred? is #t, otherwise evaluates ...else", lnfIf);
	lAddNativeFunc(c,"cond",           "[...c]",              "Contain at least 1 cond block of form (pred? ...body) and evaluates and returns the first where pred? is #t",lnfCond);
	lAddNativeFunc(c,"when",           "[condition ...body]", "Evaluates BODY if CONDITION is #t",lnfWhen);
	lAddNativeFunc(c,"unless",         "[condition ...body]", "Evaluates BODY if CONDITION is #f",lnfUnless);
	lAddNativeFunc(c,"and &&", "[...args]", "#t if all ARGS evaluate to true",            lnfAnd);
	lAddNativeFunc(c,"or ||" , "[...args]", "#t if one member of ARGS evaluates to true", lnfOr);
	lAddNativeFunc(c,"not !",  "[val]",     "#t if VAL is #f, #f if VAL is #t",           lnfNot);
}
