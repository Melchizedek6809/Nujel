 /* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../reader.h"
#include "../type/tree.h"
#include "../allocation/symbol.h"
#include "../type/closure.h"
#include "../type/val.h"
#include "../compatibility/getmsecs.h"

#include <time.h>

static lVal *lnfSymbolTable(lClosure *c, lVal *v){
	(void)v;
	lVal *l = lTreeKeysToList(c->data);
	for(lVal *n = l;n;n = n->vList.cdr){
		if(n->vList.car == NULL){break;}
		n->vList.car->type = ltSymbol;
	}
	return l;
}

static lVal *lnfClosure(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL)
		|| !((car->type == ltNativeFunc)
		||   (car->type == ltLambda)
		||   (car->type == ltObject)
		||   (car->type == ltMacro))){
		return NULL;
	}
	lVal *ret = lValTree(NULL);
	ret->vTree = lTreeInsert(ret->vTree, lSymS("type"), lValSymS(getTypeSymbol(car)));
	if(car->type == ltNativeFunc){
		lNFunc *nf = car->vNFunc;
		if(nf == NULL){return ret;}
		ret->vTree = lTreeInsert(ret->vTree, symArguments,nf->args);
		ret->vTree = lTreeInsert(ret->vTree, lSymS("name"),lValSymS(nf->name));
	}else{
		lClosure *clo = car->vClosure;
		if(clo == NULL){return ret;}
		ret->vTree = lTreeInsert(ret->vTree, symArguments, clo->args);
		ret->vTree = lTreeInsert(ret->vTree, lSymS("name"), lValSymS(clo->name));
		lVal *text = lValAlloc(ltBytecodeArr);
		text->vBytecodeArr = clo->text;
		ret->vTree = lTreeInsert(ret->vTree, lSymS("code"), text);
		ret->vTree = lTreeInsert(ret->vTree, lSymS("data"), lValTree(clo->data));
		if(clo->type == closureCall){
			ret->vTree = lTreeInsert(ret->vTree, lSymS("call"), lValBool(true));
		}
	}
	return ret;
}

static lVal *lnfClosureParent(lClosure *c, lVal *v){
	lClosure *cc = requireClosure(c, lCar(v));
	if(cc->parent == NULL){
		return NULL;
	}else{
		lVal *ret = lValAlloc(cc->parent->type == closureObject ? ltObject : ltLambda);
		ret->vClosure = cc->parent;
		return ret;
	}
}

static lVal *lnfClosureCaller(lClosure *c, lVal *v){
	lClosure *cc = requireClosure(c, lCar(v));
	if(cc->caller == NULL){
		return NULL;
	}else{
		lVal *ret = lValAlloc(cc->caller->type == closureObject ? ltObject : ltLambda);
		ret->vClosure = cc->caller;
		return ret;
	}
}

static lVal *lnfResolve(lClosure *c, lVal *v){
	const lSymbol *sym = requireSymbol(c, lCar(v));
	lVal *env = lCadr(v);
	if(env && (env->type != ltLambda) && (env->type != ltObject)){
		lExceptionThrowValClo("invalid-environment", "You can only resolve symbols in Lambdas or Objects", env, c);
	}
	return lGetClosureSym(env ? env->vClosure : c, sym);
}

static lVal *lnfResolvesPred(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	if(!car || (car->type != ltSymbol)){return lValBool(false);}
	const lSymbol *sym = car->vSymbol;
	lVal *env = lCadr(v);
	if(env && (env->type != ltLambda) && (env->type != ltObject)){
		lExceptionThrowValClo("invalid-environment", "You can only resolve symbols in Lambdas or Objects", env, c);
	}
	return lValBool(lHasClosureSym(env ? env->vClosure : c, sym,NULL));
}

static lVal *lnfCurrentClosure(lClosure *c, lVal *v){
	(void)v;
	lVal *ret = lValAlloc(ltObject);
	ret->vClosure = c;
	return ret;
}

static lVal *lnfCurrentLambda(lClosure *c, lVal *v){
	(void)v;
	lVal *ret = lValAlloc(ltLambda);
	ret->vClosure = c;
	return ret;
}

static lVal *lnfApply(lClosure *c, lVal *v){
	return lApply(c, lCadr(v), lCar(v));
}

static lVal *lnfMacroApply(lClosure *c, lVal *v){
	lVal *fun = lCar(v);
	if((fun == NULL) || (fun->type != ltMacro)){ lExceptionThrowValClo("type-error", "Can't macro-apply to that", v, c); }
	return lLambda(c, lCadr(v), fun);
}

static lVal *lnfCar(lClosure *c, lVal *v){
	(void)c;
	return lCaar(v);
}

static lVal *lnfCdr(lClosure *c, lVal *v){
	(void)c;
	return lCdar(v);
}

static lVal *lnfCons(lClosure *c, lVal *v){
	if(lCddr(v) != NULL){ lExceptionThrowValClo("too-many-args","Cons should only be called with 2 arguments!", v, c); }
	return lCons(lCar(v),lCadr(v));
}

static lVal *lnfNReverse(lClosure *c, lVal *v){
	(void)c;
	lVal *t = NULL, *l = lCar(v);
	while((l != NULL) && (l->type == ltPair)){
		lVal *next = l->vList.cdr;
		l->vList.cdr = t;
		t = l;
		l = next;
	}
	return t;
}

static lVal *lnfTime(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(time(NULL));
}

static lVal *lnfTimeMsecs(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(getMSecs());
}

static lVal *lnfLess(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lValGreater(lCar(v), lCadr(v)) < 0);
}

static lVal *lnfUnequal(lClosure *c, lVal *v){
	(void)c;
	return lValBool(!lValEqual(lCar(v), lCadr(v)));
}

static lVal *lnfEqual(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lValEqual(lCar(v), lCadr(v)));
}

static lVal *lnfLessEqual(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	return lValBool(lValEqual(a,b) || (lValGreater(a, b) < 0));
}

static lVal *lnfGreater(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lValGreater(lCar(v), lCadr(v)) > 0);
}

static lVal *lnfGreaterEqual(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	return lValBool(lValEqual(a, b) || (lValGreater(a, b) > 0));
}

static lVal *lnfNilPred(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lCar(v) == NULL);
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	if(v->type != ltPair){ lExceptionThrowValClo("invalid-quote","Quote needs a second argument to return, maybe you were trying to use a dotted pair instead of a list?", v, c); }
	return lCar(v);
}

static lVal *lnfThrow(lClosure *c, lVal *v){
	(void)c;
	lExceptionThrowRaw(lCar(v));
	return NULL;
}

static lVal *lnfRead(lClosure *c, lVal *v){
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	lString *dup = lStringDup(t->vString);
	readClosure = c;
	t = lReadList(dup,true);
	readClosure = NULL;
	return t;
}

static lVal *lnfTypeOf(lClosure *c, lVal *v){
	(void)c;
	return lValKeywordS(getTypeSymbol(lCar(v)));
}

static lVal *lnfGarbageCollect(lClosure *c, lVal *v){
	(void)c; (void)v;
	lGarbageCollect();
	return NULL;
}

static lVal *lnfClosureMetaGet(lClosure *c, lVal *v){
	lClosure *clo = requireClosure(c, lCar(v));
	const lSymbol *key = requireSymbolic(c, lCadr(v));
	return lTreeGet(clo->meta, key, NULL);
}

static lVal *lnfClosureMetaSet(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	lClosure *clo = requireClosure(c, car);
	const lSymbol *key = requireSymbolic(c, lCadr(v));
	clo->meta = lTreeInsert(clo->meta, key, lCaddr(v));
	return car;
}

static lVal *lCastFloat(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	default:      throwTypeError(c, v, ltFloat);
	case ltFloat: return v;
	case ltInt:   return lValFloat(v->vInt);
	}
}
static lVal *lnfFloat(lClosure *c, lVal *v){
	return lCastFloat(c,lCar(v));
}

static lVal *lCastInt(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	default:      throwTypeError(c, v, ltInt);
	case ltInt:   return v;
	case ltFloat: return lValInt(v->vFloat);
	}
}
static lVal *lnfInt(lClosure *c, lVal *v){
	return lCastInt(c, lCar(v));
}

static lVal *lnfBool(lClosure *c, lVal *v){
	(void)c;
	return lValBool(castToBool(lCar(v)));
}

static lVal *lnfSymbolToKeyword(lClosure *c, lVal *v){
	return lValKeywordS(requireSymbol(c, lCar(v)));
}

static lVal *lnfKeywordToSymbol(lClosure *c, lVal *v){
	return lValSymS(requireKeyword(c, lCar(v)));
}

lVal *lnfVec(lClosure *c, lVal *v){
	vec nv = vecNew(0,0,0);
	if(v == NULL){return lValVec(nv);}
	if(v->type != ltPair){
		if(v->type == ltInt){
			return lValVec(vecNew(v->vInt, v->vInt, v->vInt));
		}else if(v->type == ltFloat){
			return lValVec(vecNew(v->vFloat, v->vFloat, v->vFloat));
		}else if(v->type == ltVec){
			return v;
		}
	}
	int i = 0;
	for(lVal *cv = v; cv && cv->type == ltPair; cv = cv->vList.cdr){
		lVal *t = lCar(cv);
		if(t == NULL){break;}
		switch(t->type){
		case ltInt:
			nv.v[i] = t->vInt;
			break;
		case ltFloat:
			nv.v[i] = t->vFloat;
			break;
		case ltVec:
			if(i == 0){return t;}
			lExceptionThrowValClo("type-error", "vectors can't contain other vectors, only :float and :int values", t, c);
		default:
			lExceptionThrowValClo("type-error", "Unexpected value in [vec]", t, c);
			break;
		}
		if(++i >= 3){break;}
	}
	for(int ii=MAX(1,i);ii<3;ii++){
		nv.v[ii] = nv.v[ii-1];
	}
	return lValVec(nv);
}

void lOperationsCore(lClosure *c){
	lAddNativeFunc(c,"quote",   "[v]",   "Return v as is without evaluating", lnfQuote);
	lAddNativeFunc(c,"throw",   "[v]",   "Throw V to the closest exception handler", lnfThrow);
	lAddNativeFunc(c,"read",    "[str]", "Read and Parses STR as an S-Expression", lnfRead);

	lAddNativeFunc(c,"resolve",        "[sym environment]", "Resolve SYM until it is no longer a symbol", lnfResolve);
	lAddNativeFunc(c,"resolves?",      "[sym environment]", "Check if SYM resolves to a value",           lnfResolvesPred);

	lAddNativeFunc(c,"closure/meta",  "[clo key]",      "Retrieve KEY from CLO's metadata", lnfClosureMetaGet);
	lAddNativeFunc(c,"closure/meta!", "[clo key value]", "Set KEY to VALUE in CLO's metadata", lnfClosureMetaSet);

	lAddNativeFunc(c,"closure",        "[clo]",         "Return a tree with data about CLO",          lnfClosure);
	lAddNativeFunc(c,"closure/parent", "[clo]",         "Return the parent of CLO",                   lnfClosureParent);
	lAddNativeFunc(c,"closure/caller", "[clo]",         "Return the caller of CLO",                   lnfClosureCaller);
	lAddNativeFunc(c,"current-closure","[]",            "Return the current closure as an object",    lnfCurrentClosure);
	lAddNativeFunc(c,"current-lambda", "[]",            "Return the current closure as a lambda",     lnfCurrentLambda);

	lAddNativeFunc(c,"symbol-table*",  "[]",            "Return a list of all symbols defined, accessible from the current closure",lnfSymbolTable);

	lAddNativeFunc(c,"apply",       "[func list]",  "Evaluate FUNC with LIST as arguments",  lnfApply);
	lAddNativeFunc(c,"macro-apply", "[macro list]", "Evaluate MACRO with LIST as arguments", lnfMacroApply);

	lAddNativeFunc(c,"car",  "[list]",     "Returs the head of LIST",          lnfCar);
	lAddNativeFunc(c,"cdr",  "[list]",     "Return the rest of LIST",          lnfCdr);
	lAddNativeFunc(c,"cons", "[car cdr]",  "Return a new pair of CAR and CDR", lnfCons);
	lAddNativeFunc(c,"nreverse","[list]",  "Return LIST in reverse order, fast but mutates", lnfNReverse);

	lAddNativeFunc(c,"time",   "          []", "Return the current unix time",lnfTime);
	lAddNativeFunc(c,"time/milliseconds","[]", "Return monotonic msecs",lnfTimeMsecs);

	lAddNativeFunc(c,"<",        "[α β]", "Return true if α is less than β",             lnfLess);
	lAddNativeFunc(c,"<=",       "[α β]", "Return true if α is less or equal to β",      lnfLessEqual);
	lAddNativeFunc(c,"==",       "[α β]", "Return true if α is equal to β",              lnfEqual);
	lAddNativeFunc(c,"!=",       "[α β]", "Return true if α is not equal to  β",         lnfUnequal);
	lAddNativeFunc(c,">=",       "[α β]", "Return true if α is greater or equal than β", lnfGreaterEqual);
	lAddNativeFunc(c,">",        "[α β]", "Return true if α is greater than β",          lnfGreater);
	lAddNativeFunc(c,"nil?",     "[α]",   "Return true if α is #nil",                    lnfNilPred);

	lAddNativeFunc(c,"garbage-collect", "[]", "Force the garbage collector to run", lnfGarbageCollect);

	lAddNativeFunc(c,"type-of",         "[α]",     "Return a symbol describing the type of α", lnfTypeOf);
	lAddNativeFunc(c,"bool",            "[α]",     "Convert α into a boolean value, true or false", lnfBool);
	lAddNativeFunc(c,"int",             "[α]",     "Convert α into an integer number", lnfInt);
	lAddNativeFunc(c,"float",           "[α]",     "Convert α into a floating-point number", lnfFloat);
	lAddNativeFunc(c,"vec",             "[x y z]", "Convert α into a vector value consisting of 3 floats x,y and z", lnfVec);
	lAddNativeFunc(c,"string",          "[α]",     "Convert α into a printable and readable string", lnfCat);
	lAddNativeFunc(c,"symbol->keyword", "[α]",     "Convert symbol α into a keyword", lnfSymbolToKeyword);
	lAddNativeFunc(c,"keyword->symbol", "[α]",     "Convert keyword α into a symbol", lnfKeywordToSymbol);
}
