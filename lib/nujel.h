/*
 | nujel.h
 |
 | This file contains the public interface to the Nujel runtime.
 | While it is still not stable, it shouldn't change as often anymore.
 */
#ifndef NUJEL_LIB_NUJEL_PUBLIC
#define NUJEL_LIB_NUJEL_PUBLIC

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(_MSC_VER)
#define likely(x)   x
#define unlikely(x) x
#else
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define countof(x) (sizeof(x)/sizeof(*x))

/*
 | Now for some type/struct definitions
 */

typedef unsigned int     uint;

typedef uint64_t          u64;
typedef uint32_t          u32;
typedef uint16_t          u16;
typedef uint8_t            u8;

typedef  int64_t          i64;
typedef  int32_t          i32;
typedef  int16_t          i16;
typedef  int8_t            i8;

/*
 | And some Windows workarounds
 */
#ifdef _MSC_VER
typedef  int64_t ssize_t;
#endif

typedef enum {
	ltNil = 0,

	ltSymbol,
	ltKeyword,
	ltBool,
	ltInt,
	ltFloat,

	ltPair,
	ltArray,
	ltTree,

	ltLambda,
	ltMacro,
	ltNativeFunc,
	ltEnvironment,

	ltString,
	ltBuffer,
	ltBufferView,
	ltBytecodeArr,

	ltFileHandle,
	ltComment,
	ltException
} lType;

typedef struct lBuffer        lBuffer;
typedef struct lBufferView    lBufferView;
typedef struct lArray         lArray;
typedef struct lClosure       lClosure;
typedef struct lThread        lThread;
typedef struct lNFunc         lNFunc;
typedef struct lSymbol        lSymbol;
typedef struct lTree          lTree;
typedef struct lTreeRoot      lTreeRoot;
typedef struct lVec           lVec;
typedef struct lVal           lVal;
typedef struct lPair          lPair;
typedef struct lBytecodeArray lBytecodeArray;
typedef uint8_t lBytecodeOp;
typedef lBuffer lString;

struct lTreeRoot {
	union {
		lTree *root;
		lTreeRoot *nextFree;
	};
};

struct lVal {
	u16 type;
	union {
		bool            vBool;
		i64             vInt;
		double          vFloat;
		lPair*          vList;
		const lSymbol * vSymbol;
		FILE *          vFileHandle;
		lBytecodeArray *vBytecodeArr;
		lArray *        vArray;
		lTreeRoot *     vTree;
		lString *       vString;
		lClosure *      vClosure;
		lNFunc *        vNFunc;
		void *          vPointer;
		lBuffer *       vBuffer;
		lBufferView *   vBufferView;
	};
};

extern lVal NIL;

static inline lVal lValAlloc(lType T, void *v){
	return (lVal){T, .vPointer = v};
}

struct lPair {
	lVal car;
	union {
		lPair *nextFree;
		lVal cdr;
	};
};

/*
 | Some pretty core Nujel procedures
 */
void      lInit    ();
lClosure *lNewRoot ();
lVal      lApply   (lVal fun, lVal args);
lClosure *lLoad    (lClosure *c, const char *expr);

const void *         lBufferData            (lBuffer *v);
void *               lBufferDataMutable     (lBuffer *v);
size_t               lBufferLength          (const lBuffer *v);
const void *         lBufferViewData        (lBufferView *v);
void *               lBufferViewDataMutable (lBufferView *v);
size_t               lBufferViewLength      (const lBufferView *v);

lPair *lPairAllocRaw();
static inline lVal lCons(lVal car, lVal cdr){
	lPair *cons = lPairAllocRaw();
	cons->car = car;
	cons->cdr = cdr;
	return lValAlloc(ltPair, cons);
}

static inline lVal lCar(lVal v){
	return likely(v.type == ltPair) ? v.vList->car : NIL;
}

static inline lVal lCdr(lVal v){
	return likely(v.type == ltPair) ? v.vList->cdr : NIL;
}

static inline lVal lCaar  (lVal v){return lCar(lCar(v));}
static inline lVal lCadr  (lVal v){return lCar(lCdr(v));}
static inline lVal lCdar  (lVal v){return lCdr(lCar(v));}
static inline lVal lCddr  (lVal v){return lCdr(lCdr(v));}
static inline lVal lCaddr (lVal v){return lCar(lCdr(lCdr(v)));}
static inline lVal lCadddr(lVal v){return lCar(lCdr(lCdr(lCdr(v))));}

lVal  lValException         (const char *symbol, const char *error, lVal v);

/*
 | Reader/Printer
 */
lVal lRead(lClosure *c, const char *str);

/*
 | Type related procedores
 */
i64             castToInt        (const lVal v, i64 fallback);
bool            castToBool       (const lVal v);
const char *    castToString     (const lVal v, const char *fallback);

lVal            lValExceptionType       (lVal v, lType T);
lVal            lValExceptionArity      (lVal v, int arity);
lVal            lValExceptionNonNumeric (lVal v);
lVal            lValExceptionFloat      (lVal v);
lVal            requireInt              (lVal v);
lVal            requireNaturalInt       (lVal v);
lVal            requireFloat            (lVal v);
lVal            optionalSymbolic        (lVal v, const lSymbol *fallback);
lVal            requireSymbol           (lVal v);
lVal            requireKeyword          (lVal v);
lVal            requireSymbolic         (lVal v);
lVal            requireFileHandle       (lVal v);
lVal            requireArray            (lVal v);
lVal            requireMutableArray     (lVal v);
lVal            requireTree             (lVal v);
lVal            requireMutableTree      (lVal v);
lVal            requireString           (lVal v);
lVal            requireCallable         (lVal v);
lVal            requirePair             (lVal v);
lVal            requireBuffer           (lVal v);
lVal            requireMutableBuffer    (lVal v);
lVal            requireBufferView       (lVal v);
lVal            requireMutableBufferView(lVal v);

/*
 | Closure related procedores
 */
lVal      lDefineAliased     (lClosure *c, lVal lNF, const char *sym);

lVal      lGetClosureSym     (lClosure *c, const lSymbol *s);
void      lDefineClosureSym  (lClosure *c, const lSymbol *s, lVal v);
bool      lSetClosureSym     (lClosure *c, const lSymbol *s, lVal v);
void      lDefineVal         (lClosure *c, const char *str,  lVal v);

lVal     lAddNativeFunc     (lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal));
lVal     lAddNativeFuncFold (lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal));
lVal     lAddNativeFuncPure (lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal));
lVal     lAddNativeFuncPureFold (lClosure *c, const char *sym, const char *args, const char *doc, lVal (*func)(lClosure *,lVal));

/*
 | Tree related procedures
 */
lTree *lTreeNew             (const lSymbol *s, lVal v);
lTree *lTreeDup             (const lTree *t);

lVal   lTreeRef             (const lTree *t, const lSymbol *s);
lTree *lTreeInsert          (      lTree *t, const lSymbol *s, lVal v);

/*
 | Symbolic routines
 */
lSymbol  *lSymS         (const char *s);
lSymbol  *lSymSM        (const char *s);
lSymbol  *lSymSL        (const char *s, uint len);

/*
 | lVal related procedures
 */
static inline lVal lValFloat(double v){
	if(unlikely(isnan(v))){
		return lValException("float-nan","NaN is disallowed in Nujel", NIL);
	}
	if(unlikely(isinf(v))){
		return lValException("float-inf","INF is disallowed in Nujel", NIL);
	}
	return (lVal){ltFloat, .vFloat = v};
}

static inline lVal lValExceptionSimple(){
	return (lVal){ltException, .vList = NULL};
}

static inline lVal lValInt(i64 v){
	return (lVal){ltInt, .vInt = v};
}

static inline lVal lValBool(bool v){
	return (lVal){ltBool, .vBool = v};
}

lTreeRoot *lTreeRootAllocRaw();
static inline lVal lValTree(lTree *v){
	lTreeRoot *root = lTreeRootAllocRaw();
	root->root = v;
	return (lVal){ltTree, .vTree = root};
}

static inline lVal lValEnvironment(lClosure *v){
	return (lVal){ltEnvironment, .vClosure = v};
}

static inline lVal lValLambda(lClosure *v){
	return (lVal){ltLambda, .vClosure = v};
}

/* Return a newly allocated nujel symbol of value S */
static inline lVal lValSymS(const lSymbol *s){
	if(unlikely(s == NULL)){return NIL;}
	return (lVal){ltSymbol, .vSymbol = s};
}

/* Return a nujel value for the symbol within S */
static inline lVal lValSym(const char *s){
	return lValSymS(lSymS(s));
}

/* Return a newly allocated nujel keyword of value S */
static inline lVal lValKeywordS(const lSymbol *s){
	if(unlikely(s == NULL)){return NIL;}
	return (lVal){ltKeyword, .vSymbol = s};
}

/* Return a nujel value for the keyword within S */
static inline lVal lValKeyword(const char *s){
	return lValKeywordS(lSymS(s));
}

static inline lVal lValFileHandle(FILE *fh){
	return (lVal){ltFileHandle, .vFileHandle = fh};
}

lVal     lValString       (const char *s);
lVal     lValStringLen    (const char *s, int len);
lVal     lValStringNoCopy (const char *s, int len);

/*
 | Allocator related procedures
 */
lArray  *lArrayAlloc  (size_t len);
lBuffer *lBufferAlloc (size_t length, bool immutable);
lString *lStringNew   (const char *str, uint len);
lString *lStringDup   (const lString *s);

#endif
