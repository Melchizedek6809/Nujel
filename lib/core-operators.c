/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <inttypes.h>
#include <time.h>

static lVal lTreeAddSymbolsToList(const lTree *t, lVal list) {
    if (unlikely((t == NULL) || (t->key == NULL))) {
        return list;
    }
    list = lTreeAddSymbolsToList(t->right, list);
    list = lCons(lValSymS(t->key), list);
    return lTreeAddSymbolsToList(t->left, list);
}

static lVal lSymbolTable(lClosure *c, lVal ret) {
    if (unlikely(c == NULL)) {
        return ret;
    }
    return lSymbolTable(c->parent, lTreeAddSymbolsToList(c->data, ret));
}

static lVal lnfSymbolTable(lClosure *c, lVal v) {
    (void)v;
    return lSymbolTable(c, NIL);
}

static lVal lnfClosureParent(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireClosure(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    lClosure *cc = car.vClosure;
    if (cc->parent == NULL) {
        return NIL;
    } else {
        return lValAlloc(cc->parent->type == closureObject ? ltEnvironment : ltLambda, cc->parent);
    }
}

static lVal lnfClosureCaller(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireClosure(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    lClosure *cc = car.vClosure;
    if ((cc == NULL) || (cc->caller == NULL)) {
        return NIL;
    } else {
        return lValAlloc(cc->caller->type == closureObject ? ltEnvironment : ltLambda, cc->caller);
    }
}

static lVal lnfClosureArguments(lClosure *c, lVal v) {
    (void)c;
    lVal cc = requireCallable(lCar(v));
    if (unlikely(cc.type == ltException)) {
        return cc;
    }
    if (cc.type == ltNativeFunc) {
        return cc.vNFunc->args;
    } else {
        return cc.vClosure->args;
    }
}

static lVal lnfClosureCode(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireClosure(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    lClosure *clo = car.vClosure;
    return lValAlloc(ltBytecodeArr, clo->text);
}

static lVal lnfClosureData(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireClosure(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    lClosure *clo = car.vClosure;
    return clo ? lValTree(clo->data) : NIL;
}

static lVal lnfDefIn(lClosure *c, lVal v) {
    (void)c;
    lVal env = lCar(v);
    if (unlikely((env.type != ltLambda) && (env.type != ltEnvironment))) {
        return lValException("invalid-environment", "You can only define symbols in Lambdas or Objects", env);
    }

    lVal cadr = requireSymbolic(lCadr(v));
    if (unlikely(cadr.type == ltException)) {
        return cadr;
    }
    const lSymbol *sym = cadr.vSymbol;

    lDefineClosureSym(env.vClosure, sym, lCaddr(v));
    return env;
}

static lVal lnfResolve(lClosure *c, lVal v) {
    lVal car = requireSymbol(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    const lSymbol *sym = car.vSymbol;
    lVal env = lCadr(v);
    if (unlikely((env.type != ltNil) && (env.type != ltLambda) && (env.type != ltEnvironment))) {
        return lValException("invalid-environment", "You can only resolve symbols in Lambdas or Objects", env);
    }
    return lGetClosureSym(env.type != ltNil ? env.vClosure : c, sym);
}

static lVal lnfResolveOrNull(lClosure *c, lVal v) {
    lVal car = requireSymbol(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    const lSymbol *sym = car.vSymbol;
    lVal env = lCadr(v);
    if (unlikely((env.type != ltNil) && (env.type != ltLambda) && (env.type != ltEnvironment))) {
        return lValException("invalid-environment", "You can only resolve-or-nil symbols in Lambdas or Objects", env);
    }
    lVal ret;
    if (lHasClosureSym(env.type != ltNil ? env.vClosure : c, sym, &ret)) {
        return ret;
    } else {
        return NIL;
    }
}

static lVal lnfResolvesPred(lClosure *c, lVal v) {
    lVal car = lCar(v);
    if (car.type != ltSymbol) {
        return lValBool(false);
    }
    const lSymbol *sym = car.vSymbol;
    lVal env = lCadr(v);
    if ((env.type != ltNil) && (env.type != ltLambda) && (env.type != ltEnvironment)) {
        return lValException("invalid-environment", "You can only check symbols in Lambdas or Objects", env);
    }
    return lValBool(lHasClosureSym(env.type != ltNil ? env.vClosure : c, sym, NULL));
}

static lVal lnfCurrentClosure(lClosure *c, lVal v) {
    (void)v;
    return lValAlloc(ltEnvironment, c);
}

static lVal lnfCurrentLambda(lClosure *c, lVal v) {
    (void)v;
    return lValAlloc(ltLambda, c);
}

static lVal lnfApply(lClosure *c, lVal v) {
    lVal args = requirePair(lCadr(v));
    if (unlikely(args.type == ltException)) {
        return args;
    }
    return lApply(c, args, lCar(v));
}

static lVal lnfCar(lClosure *c, lVal v) {
    (void)c;
    return lCaar(v);
}

static lVal lnfCdr(lClosure *c, lVal v) {
    (void)c;
    return lCdar(v);
}

static lVal lnfCons(lClosure *c, lVal v) {
    (void)c;
    if (unlikely(lCddr(v).type != ltNil)) {
        return lValException("arity-error", "Cons should only be called with 2 arguments!", v);
    }
    return lCons(lCar(v), lCadr(v));
}

static lVal lnfNReverse(lClosure *c, lVal v) {
    (void)c;
    lVal t = NIL, l = lCar(v);
    while (l.type == ltPair) {
        lVal next = l.vList->cdr;
        l.vList->cdr = t;
        t = l;
        l = next;
    }
    return t;
}

static lVal lnfTime(lClosure *c, lVal v) {
    (void)c;
    (void)v;
    return lValInt(time(NULL));
}

static lVal lnfTimeMsecs(lClosure *c, lVal v) {
    (void)c;
    (void)v;
    return lValInt(getMSecs());
}

static lVal lnfLess(lClosure *c, lVal v) {
    (void)c;
    return lValBool(lValGreater(lCar(v), lCadr(v)) < 0);
}

static lVal lnfUnequal(lClosure *c, lVal v) {
    (void)c;
    return lValBool(!lValEqual(lCar(v), lCadr(v)));
}

static lVal lnfEqual(lClosure *c, lVal v) {
    (void)c;
    return lValBool(lValEqual(lCar(v), lCadr(v)));
}

static lVal lnfLessEqual(lClosure *c, lVal v) {
    (void)c;
    lVal a = lCar(v);
    lVal b = lCadr(v);
    return lValBool(lValEqual(a, b) || (lValGreater(a, b) < 0));
}

static lVal lnfGreater(lClosure *c, lVal v) {
    (void)c;
    return lValBool(lValGreater(lCar(v), lCadr(v)) > 0);
}

static lVal lnfGreaterEqual(lClosure *c, lVal v) {
    (void)c;
    lVal a = lCar(v);
    lVal b = lCadr(v);
    return lValBool(lValEqual(a, b) || (lValGreater(a, b) > 0));
}

static lVal lnfNilPred(lClosure *c, lVal v) {
    (void)c;
    return lValBool(lCar(v).type == ltNil);
}

static lVal lnfZeroPred(lClosure *c, lVal v) {
    (void)c;
    lVal a = lCar(v);
    bool p = false;

    if (a.type == ltInt) {
        p = a.vInt == 0;
    } else if (a.type == ltFloat) {
        p = a.vFloat == 0.0;
    }

    return lValBool(p);
}

static lVal lnfQuote(lClosure *c, lVal v) {
    (void)c;
    if (unlikely(v.type != ltPair)) {
        return lValException("invalid-quote",
                             "Quote needs a second argument to return, maybe you "
                             "were trying to use a dotted pair instead of a list?",
                             v);
    }
    return lCar(v);
}

static lVal lnfRead(lClosure *c, lVal v) {
    lVal car = requireString(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    return lRead(c, car.vString->data);
}

static lVal lnfTypeOf(lClosure *c, lVal v) {
    (void)c;
    return lValKeywordS(getTypeSymbol(lCar(v)));
}

static lVal lnfGarbageCollect(lClosure *c, lVal v) {
    (void)c;
    (void)v;
    lGarbageCollect();
    return NIL;
}

static lVal lnfGarbageCollectRuns(lClosure *c, lVal v) {
    (void)c;
    (void)v;
    return lValInt(lGCRuns);
}

static lVal lnfMetaGet(lClosure *c, lVal v) {
    (void)c;
    lVal cadr = requireSymbolic(lCadr(v));
    if (unlikely(cadr.type == ltException)) {
        return cadr;
    }
    const lSymbol *key = cadr.vSymbol;
    lVal l = lCar(v);
    switch (l.type) {
    case ltNativeFunc: {
        lVal t = lTreeRef(l.vNFunc->meta, key);
        return t.type != ltException ? t : NIL;
    }
    case ltLambda:
    case ltEnvironment: {
        lVal t = lTreeRef(l.vClosure->meta, key);
        return t.type != ltException ? t : NIL;
    }
    default:
        return NIL;
    }
}

static lVal lnfMetaSet(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireCallable(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }

    lVal cadr = requireSymbolic(lCadr(v));
    if (unlikely(cadr.type == ltException)) {
        return cadr;
    }
    const lSymbol *key = cadr.vSymbol;

    if (car.type == ltNativeFunc) {
        return lValException("type-error", "Can't add new metadata to native functions", car);
    } else {
        car.vClosure->meta = lTreeInsert(car.vClosure->meta, key, lCaddr(v));
    }

    return car;
}

static lVal lCastFloat(lClosure *c, lVal v) {
    (void)c;
    if (likely(v.type == ltFloat)) {
        return v;
    }
    if (likely(v.type == ltInt)) {
        return lValFloat(v.vInt);
    }
    return lValExceptionType(v, ltFloat);
}

static lVal lnfFloat(lClosure *c, lVal v) { return lCastFloat(c, lCar(v)); }

static lVal lCastInt(lClosure *c, lVal v) {
    (void)c;
    if (likely(v.type == ltInt)) {
        return v;
    }
    if (likely(v.type == ltFloat)) {
        return lValInt(v.vFloat);
    }
    return lValExceptionType(v, ltInt);
}
static lVal lnfInt(lClosure *c, lVal v) { return lCastInt(c, lCar(v)); }

static lVal lnfSymbolToKeyword(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireSymbol(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    return lValKeywordS(car.vSymbol);
}

static lVal lnfKeywordToSymbol(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireKeyword(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    return lValSymS(car.vSymbol);
}

static i64 lValToId(lVal v) {
    switch (v.type) {
    default:
        return 0;
    case ltEnvironment:
    case ltMacro:
    case ltLambda:
        return v.vClosure - lClosureList;
    case ltBufferView:
        return v.vBufferView - lBufferViewList;
    case ltString:
    case ltBuffer:
        return v.vBuffer - lBufferList;
    case ltArray:
        return v.vArray - lArrayList;
    case ltTree:
        return v.vTree - lTreeRootList;
    case ltBytecodeArr:
        return v.vBytecodeArr - lBytecodeArrayList;
    case ltKeyword:
    case ltSymbol:
        return v.vSymbol - lSymbolList;
    case ltFileHandle:
        return fileno(v.vFileHandle);
    case ltNativeFunc:
        return v.vNFunc - lNFuncList;
    }
}

static lVal lnfValToId(lClosure *c, lVal v) {
    (void)c;
    return lValInt(lValToId(lCar(v)));
}

static lVal lnfString(lClosure *c, lVal v) {
    (void)c;
    char buf[64];
    int snret;
    lVal a = lCar(v);
    switch (a.type) {
    default:
        return lValException("type-error", "Can't convert that into a string", a);
    case ltNil:
        return lValString("");
    case ltBuffer:
        return lValStringLen(a.vBuffer->data, a.vBuffer->length);
    case ltString:
        return a;
    case ltKeyword:
        snret = snprintf(buf, sizeof(buf), ":%s", a.vSymbol->c);
        break;
    case ltSymbol:
        snret = snprintf(buf, sizeof(buf), "%s", a.vSymbol->c);
        break;
    case ltInt:
        snret = snprintf(buf, sizeof(buf), "%" PRId64, a.vInt);
        break;
    case ltFloat: {
        snret = snprintf(buf, sizeof(buf), "%f", a.vFloat);
        if (snret < 0) {
            exit(5);
        }
        buf[snret--] = 0;
        for (; (snret > 0) && (buf[snret] == '0'); snret--) {
        }
        if (buf[snret] == '.') {
            snret++;
        }
        snret++;
        break;
    }
    }
    if (snret < 0) {
        exit(5);
    }
    buf[snret] = 0;
    return lValStringLen(buf, snret);
}

static lVal lnfStrSym(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireString(lCar(v));
    if (unlikely(car.type != ltString)) {
        return car;
    }
    return lValSym(car.vString->data);
}

void lOperationsCore(lClosure *c) {
    lAddNativeFuncPure(c, "quote", "(v)", "Return v as is without evaluating", lnfQuote);
    lAddNativeFuncPure(c, "read", "(str)", "Read and Parses STR as an S-Expression", lnfRead);

    lAddNativeFunc(c, "def-in!", "(environment sym v)", "Define SYM to be V in ENVIRONMENT", lnfDefIn);
    lAddNativeFunc(c, "resolve", "(sym environment)", "Resolve SYM", lnfResolve);
    lAddNativeFunc(c, "resolve-or-nil", "(sym environment)", "Resolve SYM, or return #nil if it's undefined",
                   lnfResolveOrNull);
    lAddNativeFunc(c, "resolves?", "(sym environment)", "Check if SYM resolves to a value", lnfResolvesPred);

    lAddNativeFunc(c, "val->id", "(v)", "Generate some sort of ID value for V, mainly used in [write)", lnfValToId);
    lAddNativeFunc(c, "meta", "(v key)", "Retrieve KEY from V's metadata", lnfMetaGet);
    lAddNativeFunc(c, "meta!", "(v key meta-value)", "Set KEY to META-VALUE in V's metadata", lnfMetaSet);

    lAddNativeFunc(c, "closure/data", "(clo)", "Return the data of CLO", lnfClosureData);
    lAddNativeFunc(c, "closure/code", "(clo)", "Return the code of CLO", lnfClosureCode);
    lAddNativeFunc(c, "closure/arguments", "(clo)", "Return the argument list of CLO", lnfClosureArguments);
    lAddNativeFunc(c, "closure/parent", "(clo)", "Return the parent of CLO", lnfClosureParent);
    lAddNativeFunc(c, "closure/caller", "(clo)", "Return the caller of CLO", lnfClosureCaller);

    lAddNativeFunc(c, "current-closure", "()", "Return the current closure as an object", lnfCurrentClosure);
    lAddNativeFunc(c, "current-lambda", "()", "Return the current closure as a lambda", lnfCurrentLambda);

    lAddNativeFunc(c, "symbol-table", "()",
                   "Return a list of all symbols defined, accessible from the "
                   "current closure",
                   lnfSymbolTable);

    lAddNativeFunc(c, "apply", "(func list)", "Evaluate FUNC with LIST as arguments", lnfApply);

    lAddNativeFuncPure(c, "car", "(list)", "Return the head of LIST", lnfCar);
    lAddNativeFuncPure(c, "cdr", "(list)", "Return the rest of LIST", lnfCdr);
    lAddNativeFuncPure(c, "cons", "(car cdr)", "Return a new pair of CAR and CDR", lnfCons);
    lAddNativeFunc(c, "nreverse", "(list)", "Return LIST in reverse order, fast but mutates", lnfNReverse);

    lAddNativeFunc(c, "time", "()", "Return the current unix time", lnfTime);
    lAddNativeFunc(c, "time/milliseconds", "()", "Return monotonic msecs", lnfTimeMsecs);

    lAddNativeFuncPure(c, "<", "(α β)", "Return true if α is less than β", lnfLess);
    lAddNativeFuncPure(c, "<=", "(α β)", "Return true if α is less or equal to β", lnfLessEqual);
    lAddNativeFuncPure(c, "= ==", "(α β)", "Return true if α is equal to β", lnfEqual);
    lAddNativeFuncPure(c, "not=", "(α β)", "Return true if α is not equal to  β", lnfUnequal);
    lAddNativeFuncPure(c, ">=", "(α β)", "Return true if α is greater or equal than β", lnfGreaterEqual);
    lAddNativeFuncPure(c, ">", "(α β)", "Return true if α is greater than β", lnfGreater);
    lAddNativeFuncPure(c, "nil?", "(α)", "Return true if α is #nil", lnfNilPred);
    lAddNativeFuncPure(c, "zero?", "(α)", "Return true if α is 0", lnfZeroPred);

    lAddNativeFunc(c, "garbage-collect", "()", "Force the garbage collector to run", lnfGarbageCollect);
    lAddNativeFunc(c, "garbage-collection-runs", "()", "Return the amount of times the GC ran since runtime startup",
                   lnfGarbageCollectRuns);

    lAddNativeFuncPure(c, "type-of", "(α)", "Return a symbol describing the type of α", lnfTypeOf);
    lAddNativeFuncPure(c, "int", "(α)", "Convert α into an integer number", lnfInt);
    lAddNativeFuncPure(c, "float", "(α)", "Convert α into a floating-point number", lnfFloat);
    lAddNativeFuncPure(c, "string", "(α)", "Convert α into a printable and readable string", lnfString);
    lAddNativeFuncPure(c, "symbol->keyword", "(α)", "Convert symbol α into a keyword", lnfSymbolToKeyword);
    lAddNativeFuncPure(c, "keyword->symbol", "(α)", "Convert keyword α into a symbol", lnfKeywordToSymbol);
    lAddNativeFuncPure(c, "string->symbol", "(str)", "Convert STR to a symbol", lnfStrSym);
}
