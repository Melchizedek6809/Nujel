/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <time.h>
#include <inttypes.h>

static lVal lnfClosureParent(lVal a){
	reqClosure(a);
	if(a.vClosure->parent == NULL){
		return NIL;
	}else{
		return lValAlloc(a.vClosure->parent->type == closureObject ? ltEnvironment : ltLambda, a.vClosure->parent);
	}
}

static lVal lnfClosureParentSet(lVal a, lVal parent){
	reqClosure(a);
	if(parent.type == ltNil){
		a.vClosure->parent = NULL;
	} else {
		reqClosure(parent);
		a.vClosure->parent = parent.vClosure;
	}
	return a;
}

static lVal lnfClosureArguments(lVal a){
	switch(a.type){
	case ltException:
		return a;
	case ltNativeFunc:
		return a.vNFunc->args;
	case ltLambda:
	case ltEnvironment:
	case ltMacro:
		return a.vClosure->args;
	default:
		return lValExceptionType(a, ltLambda);
	}
}

static lVal lnfClosureCode(lVal a){
	reqClosure(a);
	return lValAlloc(ltBytecodeArr, a.vClosure->text);
}

static lVal lnfClosureData(lVal a){
	reqClosure(a);
	return lValTree(a.vClosure->data);
}

static lVal lnfResolvesPred(lClosure *c, lVal aSym, lVal env){
	lVal car = aSym;
	if(unlikely(car.type != ltSymbol)){return lValBool(false);}
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

static lVal lnfUnequal(lVal a, lVal b){
	return lValBool(!lValEqual(a, b));
}

static lVal lnfQuote(lVal v){
	return v;
}

static lVal lnfRead(lVal a){
	reqString(a);
	return lRead(a.vString->data, a.vString->length);
}

static lVal lnfGarbageCollect(){
	lGarbageCollect();
	return NIL;
}

static lVal lnfGarbageCollectRuns(){
	return lValInt(lGCRuns);
}

static lVal lnmNativeMetaGet(lVal self, lVal key){
	reqSymbolic(key);
	lVal t = lTreeRef(self.vNFunc->meta, key.vSymbol);
	return t.type != ltException ? t : NIL;
}

static lVal lnmNujelMetaGet(lVal self, lVal key){
	reqSymbolic(key);
	lVal t = lTreeRef(self.vClosure->meta, key.vSymbol);
	return t.type != ltException ? t : NIL;
}

static lVal lnmNujelMetaSet(lVal self, lVal key, lVal value){
	reqSymbolic(key);
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

static lVal lnfSymbolToKeyword(lVal v){
	return lValKeywordS(v.vSymbol);
}

static lVal lnfStringToKeyword(lVal v){
	return lValKeyword(v.vString->data);
}

static lVal lnfKeywordToSymbol(lVal a){
	return lValSymS(a.vSymbol);
}

static lVal lnfStringToSymbol(lVal a){
	return lValSym(a.vString->data);
}

static lVal lnfIdentity(lVal a){
	return a;
}

static lVal lnfNilToString(lVal a){
	(void)a;
	return lValStringLen("",0);
}

static lVal lnfBufferToString(lVal a){
	return lValStringLen(a.vBuffer->data, a.vBuffer->length);
}

static lVal lnfSymbolToString(lVal a){
	return lValString(a.vSymbol->c);
}

static lVal lnfIntToString(lVal a){
	char buf[32];
	const int snret = snprintf(buf, sizeof(buf),"%" PRId64, a.vInt);
	return lValStringLen(buf,snret);
}

static lVal lnfFloatToString(lVal a){
	char buf[32];
	int snret = snprintf(buf,sizeof(buf),"%f", a.vFloat);
	if(snret < 0){
		return lValException(lSymIOError, "Unprintable flonum", a);
	}
	buf[snret--] = 0;
	for(;(snret > 0) && (buf[snret] == '0');snret--){}
	if(buf[snret] == '.'){snret++;}
	snret++;
	return lValStringLen(buf, snret);
}

void lOperationsCore(lClosure *c){
	lAddNativeFuncV(c,"quote", "(v)",   "Return v as is without evaluating", lnfQuote, NFUNC_PURE);
	lAddNativeFuncV(c,"read",  "(str)", "Read and Parses STR as an S-Expression", lnfRead, NFUNC_PURE);

	lAddNativeFuncCVV(c,"resolves?",     "(sym environment)",   "Check if SYM resolves to a value", lnfResolvesPred, 0);

	lAddNativeFuncV  (c,"val->id", "(v)", "Generate some sort of ID value for V, mainly used in (write)", lnfValToId, 0);

	lAddNativeFuncV(c, "closure/data",     "(clo)",  "Return the data of CLO",          lnfClosureData, 0);
	lAddNativeFuncV(c, "closure/code",     "(clo)",  "Return the code of CLO",          lnfClosureCode, 0);
	lAddNativeFuncV(c, "closure/arguments","(clo)",  "Return the argument list of CLO", lnfClosureArguments, 0);
	lAddNativeFuncV(c, "closure/parent",   "(clo)",  "Return the parent of CLO",        lnfClosureParent, 0);
	lAddNativeFuncVV(c,"closure/parent!", "(clo parent)",  "Set the parent of CLO",     lnfClosureParentSet, 0);

	lAddNativeFuncC(c,"current-closure", "()", "Return the current closure as an object", lnfCurrentClosure, 0);
	lAddNativeFuncC(c,"current-lambda",  "()", "Return the current closure as a lambda",  lnfCurrentLambda, 0);

	lAddNativeFuncV (c,"nreverse","(list)",    "Return LIST in reverse order, fast but mutates", lnfNReverse, 0);

	lAddNativeFunc(c,"time",             "()", "Return the current unix time",lnfTime, 0);
	lAddNativeFunc(c,"time/milliseconds","()", "Return monotonic msecs",lnfTimeMsecs, 0);

	lAddNativeFuncVV(c,"not=",     "(α β)", "Return true if α is not equal to  β", lnfUnequal, NFUNC_PURE);

	lAddNativeFunc(c,"garbage-collect",         "()", "Force the garbage collector to run", lnfGarbageCollect, 0);
	lAddNativeFunc(c,"garbage-collection-runs", "()", "Return the amount of times the GC ran since runtime startup", lnfGarbageCollectRuns, 0);

	lAddNativeFuncV(c,"int",   "(α)", "Convert α into an integer number", lnfInt, NFUNC_PURE);
	lAddNativeFuncV(c,"float", "(α)", "Convert α into a floating-point number", lnfFloat, NFUNC_PURE);

	lAddNativeFuncR(c, "array/new", "args",  "Create a new array from ...ARGS", lnfArrNew, 0);
	lAddNativeFuncR(c, "tree/new",  "plist", "Return a new tree", lnfTreeNew, 0);

	lAddNativeMethodV(&lClassList[ltNil],     lSymLTString, "(self)", lnfNilToString, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltInt],     lSymLTString, "(self)", lnfIntToString, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltFloat],   lSymLTString, "(self)", lnfFloatToString, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltBuffer],  lSymLTString, "(self)", lnfBufferToString, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltKeyword], lSymLTString, "(self)", lnfSymbolToString, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltSymbol],  lSymLTString, "(self)", lnfSymbolToString, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltString],  lSymLTString, "(self)", lnfIdentity, NFUNC_PURE);

	lAddNativeMethodV(&lClassList[ltString],  lSymLTKeyword, "(self)", lnfStringToKeyword, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltKeyword], lSymLTKeyword, "(self)", lnfIdentity, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltSymbol],  lSymLTKeyword, "(self)", lnfSymbolToKeyword, NFUNC_PURE);

	lAddNativeMethodV(&lClassList[ltString],  lSymLTSymbol, "(self)", lnfStringToSymbol, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltKeyword], lSymLTSymbol, "(self)", lnfKeywordToSymbol, NFUNC_PURE);
	lAddNativeMethodV(&lClassList[ltSymbol],  lSymLTSymbol, "(self)", lnfIdentity, NFUNC_PURE);

	lAddNativeMethodVV(&lClassList[ltNativeFunc], lSymS("meta"),  "(self key)", lnmNativeMetaGet, 0);
	lAddNativeMethodVV(&lClassList[ltLambda],     lSymS("meta"),  "(self key)", lnmNujelMetaGet, 0);
	lAddNativeMethodVVV(&lClassList[ltLambda],    lSymS("meta!"), "(self key value)", lnmNujelMetaSet, 0);
}
