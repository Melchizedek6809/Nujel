/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <time.h>
#include <inttypes.h>

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

static lVal lnfQuote(lVal v){
	return v;
}

static lVal lnfRead(lVal a){
	reqString(a);
	return lRead(a.vString->data, a.vString->length);
}

static lVal lnfGarbageCollectRuns(){
	return lValInt(lGCRuns);
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
	case ltMap: return v.vMap - lMapList;
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

lVal lValBytecodeArray(const lBytecodeOp *ops, int opsLength, lArray *literals){
	lVal ret = lValAlloc(ltBytecodeArr, lBytecodeArrayAlloc(opsLength));
	ret.vBytecodeArr->literals = literals;
	ret.vBytecodeArr->literals->flags |= ARRAY_IMMUTABLE;
	memcpy(ret.vBytecodeArr->data, ops, opsLength);
	return ret;
}

static lVal lnmBytecodeArrayArray(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	const int len = arr->dataEnd - arr->data;

	lVal ret = lValAlloc(ltArray, lArrayAlloc(len));
	for(int i=0;i<len;i++){
		ret.vArray->data[i] = lValInt(arr->data[i]);
	}
	return ret;
}

static lVal lnmBytecodeArrayLiterals(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	if(unlikely(arr->literals == NULL)){
		return NIL;
	} else {
		return lValAlloc(ltArray, arr->literals);
	}
}

static lVal lnmBytecodeArrayLength(lVal self){
	lBytecodeArray *arr = self.vBytecodeArr;
	return lValInt(arr->dataEnd - arr->data);
}

void lOperationsBytecode(){
	lClass *BytecodeArray = &lClassList[ltBytecodeArr];
	lAddNativeMethodV(BytecodeArray, lSymS("array"),    "(self)", lnmBytecodeArrayArray, 0);
	lAddNativeMethodV(BytecodeArray, lSymS("literals"), "(self)", lnmBytecodeArrayLiterals, 0);
	lAddNativeMethodV(BytecodeArray, lSymS("length"),   "(self)", lnmBytecodeArrayLength, 0);
}

/* Initialize the allocator and symbol table, needs to be called before as
 * soon as possible, since most procedures depend on it.*/
void lInit(){
	lSymbolInit();
	lTypesInit();
	lOperationsArithmetic();
	lOperationsBuffer();
	lOperationsSpecial();
	lOperationsCore();
	lOperationsArray();
	lOperationsTree();
	lOperationsMap();
	lOperationsBytecode();
	lOperationsString();
}

static lVal lnfSymbolTable(){
	return lValMap(lSymbolTable);
}

void lOperationsCore(){
	lAddNativeFuncV("quote", "(v)",   "Return v as is without evaluating", lnfQuote, NFUNC_PURE);
	lAddNativeFuncV("read",  "(str)", "Read and Parses STR as an S-Expression", lnfRead, NFUNC_PURE);

	lAddNativeFuncCVV("resolves?",     "(sym environment)",   "Check if SYM resolves to a value", lnfResolvesPred, 0);

	lAddNativeFuncV  ("val->id", "(v)", "Generate some sort of ID value for V, mainly used in (write)", lnfValToId, 0);

	lAddNativeFuncC("current-closure", "", "Return the current closure as an object", lnfCurrentClosure, 0);
	lAddNativeFuncC("current-lambda",  "", "Return the current closure as a lambda",  lnfCurrentLambda, 0);

	lAddNativeFuncV ("nreverse","(list)",    "Return LIST in reverse order, fast but mutates", lnfNReverse, 0);

	lAddNativeFunc("time",             "", "Return the current unix time",lnfTime, 0);
	lAddNativeFunc("time/milliseconds","", "Return monotonic msecs",lnfTimeMsecs, 0);

	lAddNativeFunc("garbage-collection-runs", "", "Return the amount of times the GC ran since runtime startup", lnfGarbageCollectRuns, 0);

	lAddNativeFuncV("int",   "(α)", "Convert α into an integer number", lnfInt, NFUNC_PURE);
	lAddNativeFuncV("float", "(α)", "Convert α into a floating-point number", lnfFloat, NFUNC_PURE);

	lAddNativeFuncR("array/new", "args",  "Create a new array from ...ARGS", lnfArrNew, 0);
	lAddNativeFuncR("tree/new",  "plist", "Return a new tree", lnfTreeNew, 0);
	lAddNativeFuncR("map/new",  "plist", "Return a new map", lnfMapNew, 0);

	lAddNativeFuncV("image/serialize",   "(val)", "Serializes val into a binary representation that can be stored", lnfSerialize, 0);
	lAddNativeFuncV("image/deserialize", "(buf)", "Deserializes buf into a value", lnfDeserialize, 0);
	lAddNativeFunc("symbol-table", "()", "Returns the global symbol table", lnfSymbolTable, 0);

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
}
