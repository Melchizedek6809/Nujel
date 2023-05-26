/*
 | nujel.h
 |
 | This file contains the public interface to the Nujel runtime.
 | While it is still not stable, it shouldn't change as often anymore.
 */
#ifndef NUJEL_LIB_NUJEL_PUBLIC
#define NUJEL_LIB_NUJEL_PUBLIC

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(_MSC_VER)
#define NORETURN
#define likely(x)   x
#define unlikely(x) x
#else
#define NORETURN __attribute__((noreturn))
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define countof(x) (sizeof(x)/sizeof(*x))
#define typeswitch(v) switch(v.type)

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
	ltString,
	ltArray,
	ltTree,

	ltLambda,
	ltMacro,
	ltNativeFunc,

	ltBuffer,
	ltBufferView,

	ltEnvironment,
	ltBytecodeOp,
	ltBytecodeArr,
	ltFileHandle,
	ltThread,
	ltComment
} lType;

typedef struct lBuffer lBuffer;
typedef struct lBufferView lBufferView;
typedef struct lArray   lArray;
typedef struct lClosure lClosure;
typedef struct lThread  lThread;
typedef struct lNFunc   lNFunc;
typedef struct lSymbol  lSymbol;
typedef struct lTree    lTree;
typedef struct lTreeRoot lTreeRoot;
typedef struct lVec     lVec;
typedef struct lVal     lVal;
typedef struct lPair    lPair;
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
	u32 type;
	union {
		bool            vBool;
		i64             vInt;
		double          vFloat;
		lBytecodeOp     vBytecodeOp;
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
lVal      lApply   (lClosure *c, lVal args, lVal fun);
lClosure *lLoad    (lClosure *c, const char *expr);

const char *         lStringData            (const lString *v);
const void *         lBufferData            (lBuffer *v);
void *               lBufferDataMutable     (lBuffer *v);
size_t               lBufferLength          (const lBuffer *v);
static inline size_t lStringLength          (const lString *v){return lBufferLength(v);}
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

void  lExceptionThrowRaw    (lVal v) NORETURN;
void  lExceptionThrowValClo (const char *symbol, const char *error, lVal v, lClosure *c) NORETURN;

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
const lSymbol * optionalSymbolic (lClosure *c, lVal v, const lSymbol *fallback);

NORETURN void   throwTypeError          (lClosure *c, lVal v, lType T);
NORETURN void   throwArityError         (lClosure *c, lVal v, int arity);
i64             requireInt              (lClosure *c, lVal v);
i64             requireNaturalInt       (lClosure *c, lVal v);
double          requireFloat            (lClosure *c, lVal v);
FILE *          requireFileHandle       (lClosure *c, lVal v);
lArray *        requireArray            (lClosure *c, lVal v);
lArray *        requireMutableArray     (lClosure *c, lVal v);
const lSymbol * requireSymbol           (lClosure *c, lVal v);
const lSymbol * requireKeyword          (lClosure *c, lVal v);
const lSymbol * requireSymbolic         (lClosure *c, lVal v);
lString *       requireString           (lClosure *c, lVal v);
lTreeRoot *     requireTree             (lClosure *c, lVal v);
lTreeRoot *     requireMutableTree      (lClosure *c, lVal v);
lVal            requireCallable         (lClosure *c, lVal v);
lVal            requirePair             (lClosure *c, lVal v);
lBuffer *       requireBuffer           (lClosure *c, lVal v);
lBuffer *       requireMutableBuffer    (lClosure *c, lVal v);
lBufferView *   requireBufferView       (lClosure *c, lVal v);
lBufferView *   requireMutableBufferView(lClosure *c, lVal v);

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

lVal   lTreeGet             (const lTree *t, const lSymbol *s, bool *found);
bool   lTreeHas             (const lTree *t, const lSymbol *s, lVal *value);

void   lTreeSet             (      lTree *t, const lSymbol *s, lVal v, bool *found);
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
static inline lVal lValFloat(lClosure *c, double v){
	if(unlikely(__builtin_isnan(v))){
		lExceptionThrowValClo("float-nan","NaN is disallowed in Nujel", NIL, c);
	}else if(unlikely(__builtin_isinf(v))){
		lExceptionThrowValClo("float-inf","INF is disallowed in Nujel", NIL, c);
	}
	return (lVal){ltFloat, .vFloat = v};
}

static inline lVal lValInt(i64 v){
	return (lVal){ltInt, .vInt = v};
}

static inline lVal lValBool(bool v){
	return (lVal){ltBool, .vBool = v};
}

lTreeRoot *lTreeRootAllocRaw();
static inline lVal lValTree(lTree *v){
	lVal ret = lValAlloc(ltTree, lTreeRootAllocRaw());
	ret.vTree->root = v;
	return ret;
}

static inline lVal lValEnvironment(lClosure *v){
	return lValAlloc(ltEnvironment, v);
}

static inline lVal lValLambda(lClosure *v){
	return lValAlloc(ltLambda, v);
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
	return lValAlloc(ltFileHandle, fh);
}

static inline lVal lValBytecodeOp(lBytecodeOp v){
	return (lVal){ltBytecodeOp, .vBytecodeOp = v};
}

lVal     lValString       (const char *s);
lVal     lValStringLen    (const char *s, int len);
lVal     lValStringNoCopy (const char *s, int len);

/*
 | Allocator related procedures
 */
lArray *         lArrayAlloc  (size_t len);
lBuffer *        lBufferAlloc (size_t length, bool immutable);
lString *        lStringNew   (const char *str, uint len);
lString *        lStringDup   (const lString *s);

#endif
