 /* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

#include <time.h>
#include <inttypes.h>

static lVal *lnfSymbolTable(lClosure *c, lVal *v){
	(void)v;
	lVal *l = lTreeKeysToList(c->data);
	for(lVal *n = l;n;n = n->vList.cdr){
		if(n->vList.car == NULL){break;}
		n->vList.car->type = ltSymbol;
	}
	return l;
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
	if((cc == NULL) || (cc->caller == NULL)){
		return NULL;
	}else{
		lVal *ret = lValAlloc(cc->caller->type == closureObject ? ltObject : ltLambda);
		ret->vClosure = cc->caller;
		return ret;
	}
}

static lVal *lnfClosureArguments(lClosure *c, lVal *v){
	lVal *cc = requireCallable(c, lCar(v));
	if(cc->type == ltNativeFunc){
		return cc->vNFunc->args;
	}else{
		return cc->vClosure->args;
	}
}

static lVal *lnfClosureCode(lClosure *c, lVal *v){
	lClosure *clo = requireClosure(c, lCar(v));
	lVal *text = lValAlloc(ltBytecodeArr);
	text->vBytecodeArr = clo->text;
	return text;
}

static lVal *lnfClosureData(lClosure *c, lVal *v){
	lClosure *clo = requireClosure(c, lCar(v));
	return clo ? lValTree(clo->data) : NULL;
}

static lVal *lnfClosureName(lClosure *c, lVal *v){
	lVal *cc = requireCallable(c, lCar(v));
	return lValSymS((cc->type == ltNativeFunc) ? cc->vNFunc->name : cc->vClosure->name);
}

static lVal *lnfDefIn(lClosure *c, lVal *v){
	const lSymbol *sym = requireSymbol(c, lCadr(v));
	lVal *env = lCar(v);
	if(!env || ((env->type != ltLambda) && (env->type != ltObject))){
		lExceptionThrowValClo("invalid-environment", "You can only resolve symbols in Lambdas or Objects", env, c);
	}
	lDefineClosureSym(env->vClosure, sym, lCaddr(v));
	return env;
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
	return lApply(c, requirePair(c, lCadr(v)), lCar(v));
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

static lVal *lnfZeroPred(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	bool p = false;
	if(a){
		if(a->type == ltInt){
			p = a->vInt == 0;
		}else if(a->type == ltFloat){
			p = a->vFloat == 0.0;
		}
	}
	return lValBool(p);
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
	return lRead(c, requireString(c, lCar(v))->data);
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

static lVal *lnfGarbageCollectRuns(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(lGCRuns);
}

static lVal *lnfMetaGet(lClosure *c, lVal *v){
	const lSymbol *key = requireSymbolic(c, lCadr(v));
	lVal *l = lCar(v);
	typeswitch(l){
	case ltNativeFunc:
		return lTreeGet(l->vNFunc->meta, key, NULL);
	case ltLambda:
	case ltObject:
		return lTreeGet(l->vClosure->meta, key, NULL);
	default:
		return NULL;
	}
}

static lVal *lnfMetaSet(lClosure *c, lVal *v){
	const lSymbol *key = requireSymbolic(c, lCadr(v));
	lVal *car = requireCallable(c, lCar(v));

	if(car->type == ltNativeFunc){
		lExceptionThrowValClo("type-error", "Can't add new metadata to native functions", car, c);
	}else{
		car->vClosure->meta = lTreeInsert(car->vClosure->meta, key, lCaddr(v));
	}

	return car;
}

static lVal *lCastFloat(lClosure *c, lVal *v){
	typeswitch(v){
	default:      throwTypeError(c, v, ltFloat);
	case ltFloat: return v;
	case ltInt:   return lValFloat(c, v->vInt);
	}
}
static lVal *lnfFloat(lClosure *c, lVal *v){
	return lCastFloat(c,lCar(v));
}

static lVal *lCastInt(lClosure *c, lVal *v){
	typeswitch(v){
	default:      throwTypeError(c, v, ltInt);
	case ltInt:   return v;
	case ltFloat: return lValInt(v->vFloat);
	}
}
static lVal *lnfInt(lClosure *c, lVal *v){
	return lCastInt(c, lCar(v));
}

static lVal *lnfSymbolToKeyword(lClosure *c, lVal *v){
	return lValKeywordS(requireSymbol(c, lCar(v)));
}

static lVal *lnfKeywordToSymbol(lClosure *c, lVal *v){
	return lValSymS(requireKeyword(c, lCar(v)));
}

static i64 lValToId(lVal *v){
	typeswitch(v){
	default:      return 0;
	case ltObject:
	case ltMacro:
	case ltLambda: return v->vClosure - lClosureList;
	case ltBufferView: return v->vBufferView - lBufferViewList;
	case ltString:
	case ltBuffer: return v->vBuffer - lBufferList;
	case ltArray: return v->vArray - lArrayList;
	case ltTree: return v->vTree - lTreeList;
	case ltBytecodeArr: return v->vBytecodeArr - lBytecodeArrayList;
	case ltKeyword:
	case ltSymbol: return v->vSymbol - lSymbolList;
	case ltFileHandle: return fileno(v->vFileHandle);
	case ltNativeFunc: return v->vNFunc - lNFuncList;
	}
}

static lVal *lnfValToId(lClosure *c, lVal *v){
	(void)c;
	return lValInt(lValToId(lCar(v)));
}

static lVal *lnfString(lClosure *c, lVal *v){
	char buf[64];
	int snret;
	lVal *a = lCar(v);
	typeswitch(a){
	default:
		lExceptionThrowValClo("type-error", "Can't convert that into a string", a, c);
		return NULL;
	case ltNoAlloc:
		return lValString("");
	case ltBuffer:
		return lValStringLen(a->vBuffer->data, a->vBuffer->length);
	case ltString:
		return a;
	case ltKeyword:
		snret = snprintf(buf,sizeof(buf),":%s",a->vSymbol->c);
		break;
	case ltSymbol:
		snret = snprintf(buf,sizeof(buf),"%s",a->vSymbol->c);
		break;
	case ltInt:
		snret = snprintf(buf,sizeof(buf),"%" PRId64, a->vInt);
		break;
	case ltFloat: {
		snret = snprintf(buf,sizeof(buf),"%f", a->vFloat);
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

static lVal *lnfStrSym(lClosure *c, lVal *v){
	return lValSym(requireString(c, lCar(v))->data);
}

void lOperationsCore(lClosure *c){
	lAddNativeFuncPure(c,"quote", "[v]",   "Return v as is without evaluating", lnfQuote);
	lAddNativeFuncPure(c,"read",  "[str]", "Read and Parses STR as an S-Expression", lnfRead);
	lAddNativeFunc    (c,"throw", "[v]",   "Throw V to the closest exception handler", lnfThrow);

	lAddNativeFunc(c,"def-in!",        "[environment sym v]", "Define SYM to be V in ENVIRONMENT", lnfDefIn);
	lAddNativeFunc(c,"resolve",        "[sym environment]", "Resolve SYM until it is no longer a symbol", lnfResolve);
	lAddNativeFunc(c,"resolves?",      "[sym environment]", "Check if SYM resolves to a value",           lnfResolvesPred);

	lAddNativeFunc(c,"val->id", "[v]",                "Generate some sort of ID value for V, mainly used in [write]", lnfValToId);
	lAddNativeFunc(c,"meta",    "[v key]",            "Retrieve KEY from V's metadata", lnfMetaGet);
	lAddNativeFunc(c,"meta!",   "[v key meta-value]", "Set KEY to META-VALUE in V's metadata", lnfMetaSet);

	lAddNativeFunc(c,"closure/data",     "[clo]", "Return the data of CLO",                     lnfClosureData);
	lAddNativeFunc(c,"closure/code",     "[clo]", "Return the code of CLO",                     lnfClosureCode);
	lAddNativeFunc(c,"closure/name",     "[clo]", "Return the name of CLO",                     lnfClosureName);
	lAddNativeFunc(c,"closure/arguments","[clo]", "Return the argument list of CLO",            lnfClosureArguments);
	lAddNativeFunc(c,"closure/parent",   "[clo]", "Return the parent of CLO",                   lnfClosureParent);
	lAddNativeFunc(c,"closure/caller",   "[clo]", "Return the caller of CLO",                   lnfClosureCaller);

	lAddNativeFunc(c,"current-closure",  "[]",    "Return the current closure as an object",    lnfCurrentClosure);
	lAddNativeFunc(c,"current-lambda",   "[]",    "Return the current closure as a lambda",     lnfCurrentLambda);

	lAddNativeFunc(c,"symbol-table*",  "[]",            "Return a list of all symbols defined, accessible from the current closure",lnfSymbolTable);

	lAddNativeFunc(c,"apply",       "[func list]",  "Evaluate FUNC with LIST as arguments",  lnfApply);
	lAddNativeFunc(c,"macro-apply", "[macro list]", "Evaluate MACRO with LIST as arguments", lnfMacroApply);

	lAddNativeFuncPure(c,"car",     "[list]",    "Returs the head of LIST",          lnfCar);
	lAddNativeFuncPure(c,"cdr",     "[list]",    "Return the rest of LIST",          lnfCdr);
	lAddNativeFuncPure(c,"cons",    "[car cdr]", "Return a new pair of CAR and CDR", lnfCons);
	lAddNativeFuncPure(c,"nreverse","[list]",    "Return LIST in reverse order, fast but mutates", lnfNReverse);

	lAddNativeFunc(c,"time",   "          []", "Return the current unix time",lnfTime);
	lAddNativeFunc(c,"time/milliseconds","[]", "Return monotonic msecs",lnfTimeMsecs);

	lAddNativeFuncPure(c,"<",        "[α β]", "Return true if α is less than β",             lnfLess);
	lAddNativeFuncPure(c,"<=",       "[α β]", "Return true if α is less or equal to β",      lnfLessEqual);
	lAddNativeFuncPure(c,"= ==",     "[α β]", "Return true if α is equal to β",              lnfEqual);
	lAddNativeFuncPure(c,"not=",     "[α β]", "Return true if α is not equal to  β",       lnfUnequal);
	lAddNativeFuncPure(c,">=",       "[α β]", "Return true if α is greater or equal than β", lnfGreaterEqual);
	lAddNativeFuncPure(c,">",        "[α β]", "Return true if α is greater than β",          lnfGreater);
	lAddNativeFuncPure(c,"nil?",     "[α]",   "Return true if α is #nil",                    lnfNilPred);
	lAddNativeFuncPure(c,"zero?",    "[α]",   "Return true if α is 0",                       lnfZeroPred);

	lAddNativeFunc(c,"garbage-collect",         "[]", "Force the garbage collector to run", lnfGarbageCollect);
	lAddNativeFunc(c,"garbage-collection-runs", "[]", "Return the amount of times the GC ran since runtime startup", lnfGarbageCollectRuns);

	lAddNativeFuncPure(c,"type-of",         "[α]",     "Return a symbol describing the type of α", lnfTypeOf);
	lAddNativeFuncPure(c,"int",             "[α]",     "Convert α into an integer number", lnfInt);
	lAddNativeFuncPure(c,"float",           "[α]",     "Convert α into a floating-point number", lnfFloat);
	lAddNativeFuncPure(c,"string",          "[α]",     "Convert α into a printable and readable string", lnfString);
	lAddNativeFuncPure(c,"symbol->keyword", "[α]",     "Convert symbol α into a keyword", lnfSymbolToKeyword);
	lAddNativeFuncPure(c,"keyword->symbol", "[α]",     "Convert keyword α into a symbol", lnfKeywordToSymbol);
	lAddNativeFuncPure(c,"string->symbol",  "[str]",   "Convert STR to a symbol",         lnfStrSym);
}
