/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <time.h>
#include <inttypes.h>

static lVal lTreeAddSymbolsToList(const lTree *t, lVal list){
	if(unlikely((t == NULL) || (t->key == NULL))){return list;}
	list = lTreeAddSymbolsToList(t->right, list);
	list = lCons(lValSymS(t->key), list);
	return lTreeAddSymbolsToList(t->left, list);
}

static lVal lSymbolTable(lClosure *c, lVal ret){
	if(unlikely(c == NULL)){return ret;}
	return lSymbolTable(c->parent, lTreeAddSymbolsToList(c->data, ret));
}

static lVal lnfSymbolTable(lClosure *c){
	return lSymbolTable(c, NIL);
}

static lVal lnfClosureParent(lVal a){
	lVal car = requireClosure(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lClosure *cc = car.vClosure;
	if(cc->parent == NULL){
		return NIL;
	}else{
		return lValAlloc(cc->parent->type == closureObject ? ltEnvironment : ltLambda, cc->parent);
	}
}

static lVal lnfClosureArguments(lVal a){
	lVal cc = requireCallable(a);
	if(unlikely(cc.type == ltException)){
		return cc;
	}
	if(cc.type == ltNativeFunc){
		return cc.vNFunc->args;
	}else{
		return cc.vClosure->args;
	}
}

static lVal lnfClosureCode(lVal a){
	lVal car = requireClosure(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lClosure *clo = car.vClosure;
	return lValAlloc(ltBytecodeArr, clo->text);
}

static lVal lnfClosureData(lVal a){
	lVal car = requireClosure(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lClosure *clo = car.vClosure;
	return clo ? lValTree(clo->data) : NIL;
}

static lVal lnfDefIn(lVal env, lVal aSym, lVal aVal){
	if(unlikely((env.type != ltLambda) && (env.type != ltEnvironment))){
		return lValException(lSymTypeError, "You can only define symbols in Lambdas or Objects", env);
	}

	lVal cadr = requireSymbolic(aSym);
	if(unlikely(cadr.type == ltException)){
		return cadr;
	}
	const lSymbol *sym = cadr.vSymbol;

	lDefineClosureSym(env.vClosure, sym, aVal);
	return env;
}

static lVal lnfResolve(lClosure *c, lVal aSym, lVal env){
	lVal car = requireSymbol(aSym);
	if(unlikely(car.type == ltException)){
		return car;
	}
	const lSymbol *sym = car.vSymbol;
	if(unlikely((env.type != ltNil) && (env.type != ltLambda) && (env.type != ltEnvironment))){
		return lValException(lSymTypeError, "You can only resolve symbols in Lambdas or Objects", env);
	}
	return lGetClosureSym(env.type != ltNil ? env.vClosure : c, sym);
}

static lVal lnfResolveOrNull(lClosure *c, lVal aSym, lVal env){
	lVal car = requireSymbol(aSym);
	if(unlikely(car.type == ltException)){
		return car;
	}
	const lSymbol *sym = car.vSymbol;
	if(unlikely((env.type != ltNil) && (env.type != ltLambda) && (env.type != ltEnvironment))){
		return lValException(lSymTypeError, "You can only resolve-or-nil symbols in Lambdas or Objects", env);
	}
	const lVal ret = lGetClosureSym(env.type != ltNil ? env.vClosure : c, sym);
	if(ret.type == ltException){
		return NIL;
	} else {
		return ret;
	}
}

static lVal lnfResolvesPred(lClosure *c, lVal aSym, lVal env){
	lVal car = aSym;
	if(car.type != ltSymbol){return lValBool(false);}
	const lSymbol *sym = car.vSymbol;
	if((env.type != ltNil) && (env.type != ltLambda) && (env.type != ltEnvironment)){
		return lValException(lSymTypeError, "You can only check symbols in Lambdas or Objects", env);
	}
	const lVal ret = lGetClosureSym(env.type != ltNil ? env.vClosure : c, sym);
	return lValBool(ret.type != ltException);
}

static lVal lnfCurrentClosure(lClosure *c){
	return lValAlloc(ltEnvironment, c);
}

static lVal lnfCurrentLambda(lClosure *c){
	return lValAlloc(ltLambda, c);
}

static lVal lnfCar(lVal v){
	return lCar(v);
}

static lVal lnfCdr(lVal v){
	return lCdr(v);
}

static lVal lnfCons(lVal a, lVal b){
	return lCons(a, b);
}

static lVal lnfNReverse(lVal l){
	lVal t = NIL;
	while(l.type == ltPair){
		lVal next = l.vList->cdr;
		l.vList->cdr = t;
		t = l;
		l = next;
	}
	return t;
}

static lVal lnfTime(){
	return lValInt(time(NULL));
}

static lVal lnfTimeMsecs(){
	return lValInt(getMSecs());
}

static lVal lnfLess(lVal a, lVal b){
	return lValBool(lValGreater(a, b) < 0);
}

static lVal lnfUnequal(lVal a, lVal b){
	return lValBool(!lValEqual(a, b));
}

static lVal lnfEqual(lVal a, lVal b){
	return lValBool(lValEqual(a, b));
}

static lVal lnfLessEqual(lVal a, lVal b){
	return lValBool(lValEqual(a,b) || (lValGreater(a, b) < 0));
}

static lVal lnfGreater(lVal a, lVal b){
	return lValBool(lValGreater(a, b) > 0);
}

static lVal lnfGreaterEqual(lVal a, lVal b){
	return lValBool(lValEqual(a, b) || (lValGreater(a, b) > 0));
}

static lVal lnfNilPred(lVal a){
	return lValBool(a.type == ltNil);
}

static lVal lnfZeroPred(lVal a){
	if(a.type == ltInt){
		return lValBool(a.vInt == 0);
	} else if(a.type == ltFloat){
		return lValBool(a.vFloat == 0.0);
	} else {
		return lValBool(false);
	}
}

static lVal lnfQuote(lVal v){
	return v;
}

static lVal lnfRead(lVal a){
	lVal car = requireString(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	return lRead(car.vString->data);
}

static lVal lnfTypeOf(lVal a){
	return lValKeywordS(getTypeSymbol(a));
}

static lVal lnfGarbageCollect(){
	lGarbageCollect();
	return NIL;
}

static lVal lnfGarbageCollectRuns(){
	return lValInt(lGCRuns);
}

static lVal lnmNativeMetaGet(lVal self, lVal key){
	lVal e = requireSymbolic(key);
	if(unlikely(e.type == ltException)){
		return e;
	}
	lVal t = lTreeRef(self.vNFunc->meta, key.vSymbol);
	return t.type != ltException ? t : NIL;
}

static lVal lnmNujelMetaGet(lVal self, lVal key){
	lVal e = requireSymbolic(key);
	if(unlikely(e.type == ltException)){
		return e;
	}
	lVal t = lTreeRef(self.vClosure->meta, key.vSymbol);
	return t.type != ltException ? t : NIL;
}

static lVal lnmNujelMetaSet(lVal self, lVal key, lVal value){
	lVal e = requireSymbolic(key);
	if(unlikely(e.type == ltException)){
		return e;
	}
	self.vClosure->meta = lTreeInsert(self.vClosure->meta, key.vSymbol, value);
	return self;
}

static lVal lnfFloat(lVal v){
	if(likely(v.type == ltFloat)){
		return v;
	} else if(likely(v.type == ltInt)){
		return lValFloat(v.vInt);
	} else {
		return lValExceptionType(v, ltFloat);
	}
}

static lVal lnfInt(lVal v){
	if(likely(v.type == ltInt)){
		return v;
	} else if(likely(v.type == ltFloat)){
		return lValInt(v.vFloat);
	} else {
		return lValExceptionType(v, ltInt);
	}
}

static lVal lnfSymbolToKeyword(lVal v){
	lVal car = requireSymbol(v);
	if(unlikely(car.type == ltException)){
		return car;
	}
	return lValKeywordS(car.vSymbol);
}

static lVal lnfKeywordToSymbol(lVal a){
	lVal car = requireKeyword(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	return lValSymS(car.vSymbol);
}

static i64 lValToId(lVal v){
	switch(v.type){
	default:      return 0;
	case ltEnvironment:
	case ltMacro:
	case ltLambda: return v.vClosure - lClosureList;
	case ltBufferView: return v.vBufferView - lBufferViewList;
	case ltString:
	case ltBuffer: return v.vBuffer - lBufferList;
	case ltArray: return v.vArray - lArrayList;
	case ltTree: return v.vTree - lTreeRootList;
	case ltBytecodeArr: return v.vBytecodeArr - lBytecodeArrayList;
	case ltKeyword:
	case ltSymbol: return v.vSymbol - lSymbolList;
	case ltFileHandle: return fileno(v.vFileHandle);
	case ltNativeFunc: return v.vNFunc - lNFuncList;
	}
}

static lVal lnfValToId(lVal a){
	return lValInt(lValToId(a));
}

static lVal lnfString(lVal a){
	char buf[64];
	int snret;
	switch(a.type){
	default:
		return lValException(lSymTypeError, "Can't convert that into a string", a);
	case ltNil:
		return lValString("");
	case ltBuffer:
		return lValStringLen(a.vBuffer->data, a.vBuffer->length);
	case ltString:
		return a;
	case ltKeyword:
		snret = snprintf(buf,sizeof(buf),":%s",a.vSymbol->c);
		break;
	case ltSymbol:
		snret = snprintf(buf,sizeof(buf),"%s",a.vSymbol->c);
		break;
	case ltInt:
		snret = snprintf(buf,sizeof(buf),"%" PRId64, a.vInt);
		break;
	case ltFloat: {
		snret = snprintf(buf,sizeof(buf),"%f", a.vFloat);
		if(snret < 0){exit(5);}
		buf[snret--] = 0;
		for(;(snret > 0) && (buf[snret] == '0');snret--){}
		if(buf[snret] == '.'){snret++;}
		snret++;
		break; }
	}
	if(snret < 0){exit(5);}
	buf[snret] = 0;
	return lValStringLen(buf,snret);
}

static lVal lnfStrSym(lVal a){
	lVal car = requireString(a);
	if(unlikely(car.type != ltString)){
		return car;
	}
	return lValSym(car.vString->data);
}

/*
static lVal lnfMethod(lClosure *c, lVal v){
	(void)c;(void)v;
	return lValException("reserved", "method calls aren't implemented yet", v);
}
*/

void lOperationsCore(lClosure *c){
	lAddNativeFuncV(c,"quote", "(v)",   "Return v as is without evaluating", lnfQuote, NFUNC_PURE);
	lAddNativeFuncV(c,"read",  "(str)", "Read and Parses STR as an S-Expression", lnfRead, NFUNC_PURE);

	lAddNativeFuncVVV(c,"def-in!",        "(environment sym v)", "Define SYM to be V in ENVIRONMENT", lnfDefIn, 0);
	lAddNativeFuncCVV(c,"resolve",        "(sym environment)", "Resolve SYM", lnfResolve, 0);
	lAddNativeFuncCVV(c,"resolve-or-nil","(sym environment)", "Resolve SYM, or return #nil if it's undefined", lnfResolveOrNull, 0);
	lAddNativeFuncCVV(c,"resolves?",      "(sym environment)", "Check if SYM resolves to a value",           lnfResolvesPred, 0);

	lAddNativeFuncV  (c,"val->id", "(v)",                "Generate some sort of ID value for V, mainly used in [write)", lnfValToId, 0);

	lAddNativeMethodVV(&lClassList[ltNativeFunc], lSymS("meta"),  "(self key)", lnmNativeMetaGet, 0);
	lAddNativeMethodVV(&lClassList[ltLambda],     lSymS("meta"),  "(self key)", lnmNujelMetaGet, 0);
	lAddNativeMethodVVV(&lClassList[ltLambda],    lSymS("meta!"), "(self key value)", lnmNujelMetaSet, 0);

	lAddNativeFuncV(c,"closure/data",     "(clo)",  "Return the data of CLO",                     lnfClosureData, 0);
	lAddNativeFuncV(c,"closure/code",     "(clo)",  "Return the code of CLO",                     lnfClosureCode, 0);
	lAddNativeFuncV(c,"closure/arguments","(clo)",  "Return the argument list of CLO",            lnfClosureArguments, 0);
	lAddNativeFuncV(c,"closure/parent",   "(clo)",  "Return the parent of CLO",                   lnfClosureParent, 0);

	lAddNativeFuncC(c,"current-closure",  "()",     "Return the current closure as an object",    lnfCurrentClosure, 0);
	lAddNativeFuncC(c,"current-lambda",   "()",     "Return the current closure as a lambda",     lnfCurrentLambda, 0);

	lAddNativeFuncC(c,"symbol-table",  "()",        "Return a list of all symbols defined, accessible from the current closure",lnfSymbolTable, 0);

	lAddNativeFuncV (c,"car",     "(list)",       "Return the head of LIST",          lnfCar, 0);
	lAddNativeFuncV (c,"cdr",     "(list)",       "Return the rest of LIST",          lnfCdr, 0);
	lAddNativeFuncVV(c,"cons",    "(car cdr)",    "Return a new pair of CAR and CDR", lnfCons, 0);
	lAddNativeFuncV (c,"nreverse","(list)",       "Return LIST in reverse order, fast but mutates", lnfNReverse, 0);

	lAddNativeFunc(c,"time",             "()", "Return the current unix time",lnfTime, 0);
	lAddNativeFunc(c,"time/milliseconds","()", "Return monotonic msecs",lnfTimeMsecs, 0);

	lAddNativeFuncVV(c,"<",        "(α β)", "Return true if α is less than β",             lnfLess, NFUNC_PURE);
	lAddNativeFuncVV(c,"<=",       "(α β)", "Return true if α is less or equal to β",      lnfLessEqual, NFUNC_PURE);
	lAddNativeFuncVV(c,"= ==",     "(α β)", "Return true if α is equal to β",              lnfEqual, NFUNC_PURE);
	lAddNativeFuncVV(c,"not=",     "(α β)", "Return true if α is not equal to  β",         lnfUnequal, NFUNC_PURE);
	lAddNativeFuncVV(c,">=",       "(α β)", "Return true if α is greater or equal than β", lnfGreaterEqual, NFUNC_PURE);
	lAddNativeFuncVV(c,">",        "(α β)", "Return true if α is greater than β",          lnfGreater, NFUNC_PURE);
	lAddNativeFuncV (c,"nil?",     "(α)",   "Return true if α is #nil",                    lnfNilPred, NFUNC_PURE);
	lAddNativeFuncV (c,"zero?",    "(α)",   "Return true if α is 0",                       lnfZeroPred, NFUNC_PURE);

	lAddNativeFunc(c,"garbage-collect",         "()", "Force the garbage collector to run", lnfGarbageCollect, 0);
	lAddNativeFunc(c,"garbage-collection-runs", "()", "Return the amount of times the GC ran since runtime startup", lnfGarbageCollectRuns, 0);

	lAddNativeFuncV(c,"type-of",         "(α)",     "Return a symbol describing the type of α", lnfTypeOf, NFUNC_PURE);
	lAddNativeFuncV(c,"int",             "(α)",     "Convert α into an integer number", lnfInt, NFUNC_PURE);
	lAddNativeFuncV(c,"float",           "(α)",     "Convert α into a floating-point number", lnfFloat, NFUNC_PURE);
	lAddNativeFuncV(c,"string",          "(α)",     "Convert α into a printable and readable string", lnfString, NFUNC_PURE);
	lAddNativeFuncV(c,"symbol->keyword", "(α)",     "Convert symbol α into a keyword", lnfSymbolToKeyword, NFUNC_PURE);
	lAddNativeFuncV(c,"keyword->symbol", "(α)",     "Convert keyword α into a symbol", lnfKeywordToSymbol, NFUNC_PURE);
	lAddNativeFuncV(c,"string->symbol",  "(str)",   "Convert STR to a symbol",         lnfStrSym, NFUNC_PURE);
}
