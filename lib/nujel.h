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

#define PI    (3.1415926535897932384626433832795)

#ifdef __WATCOMC__
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
#define typeswitch(v) switch(v ? v->type : ltNoAlloc)

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

struct vec {
	union {
		struct { float x,y,z,w; };
		struct { float v[4]; };
		struct { float yaw,pitch,roll,_w; };
	};
};
typedef struct vec vec;


typedef enum {
	ltNoAlloc = 0,
	ltComment = 1,

	ltSymbol = 2,
	ltKeyword = 3,
	ltBool = 4,
	ltInt = 5,
	ltFloat = 6,
	ltVec = 7,

	ltPair = 8,
	ltString = 9,
	ltArray = 10,
	ltTree = 11,

	ltLambda = 12,
	ltObject = 13,
	ltMacro = 14,
	ltThread = 15,
	ltNativeFunc = 16,
	ltBytecodeOp = 17,
	ltBytecodeArr = 18,

	ltBuffer = 19,
	ltBufferView = 20,

	ltGUIWidget
} lType;

typedef struct lBuffer lBuffer;
typedef struct lBufferView lBufferView;
typedef struct lArray   lArray;
typedef struct lClosure lClosure;
typedef struct lThread  lThread;
typedef struct lNFunc   lNFunc;
typedef struct lSymbol  lSymbol;
typedef struct lTree    lTree;
typedef struct lVec     lVec;
typedef struct lVal     lVal;
typedef struct lBytecodeArray lBytecodeArray;
typedef uint8_t lBytecodeOp;
typedef lBuffer lString;


typedef enum {
	lbvtUndefined = 0,
	lbvtS8,
	lbvtU8,
	lbvtS16,
	lbvtU16,
	lbvtS32,
	lbvtU32,
	lbvtS64,
	lbvtF32,
	lbvtF64
} lBufferViewType;

struct lBufferView {
	union {
		lBuffer *buf;
		lBufferView *nextFree;
	};
	size_t offset;
	size_t length;
	lBufferViewType type;
	u8 flags;
};
#define BUFFER_VIEW_IMMUTABLE 1

struct lBytecodeArray{
	lBytecodeOp *data;
	lArray *literals;
	union {
		lBytecodeOp *dataEnd;
		struct lBytecodeArray *nextFree;
	};
	u8 flags;
};

struct lBuffer {
	union {
		void *buf;
		const char *data;
		lBuffer *nextFree;
	};
	i32 length;
	u8 flags;
};
#define BUFFER_IMMUTABLE 1

struct lArray {
	lVal **data;
	union {
		lArray *nextFree;
		struct {
			i32 length;
			u8 flags;
		};
	};
};
#define ARRAY_IMMUTABLE 1


struct lSymbol {
	union {
		char c[32];
		struct lSymbol *nextFree;
	};
};

typedef struct {
	lVal *car,*cdr;
} lPair;

struct lVal {
	u32 type;
	union {
		bool            vBool;
		lPair           vList;
		i64             vInt;
		double          vFloat;
		vec             vVec;
		lBytecodeOp     vBytecodeOp;
		lBytecodeArray *vBytecodeArr;
		lArray         *vArray;
		lTree          *vTree;
		lString        *vString;
		const lSymbol  *vSymbol;
		lClosure       *vClosure;
		lNFunc         *vNFunc;
		void           *vPointer;
		lVal           *nextFree;
		lBuffer        *vBuffer;
		lBufferView    *vBufferView;
	};
};

typedef enum closureType {
	closureDefault = 0,
	closureObject = 1,
	closureCall = 2,
	closureLet = 3,
	closureTry = 4,
	closureRoot = 5,
} closureType;

struct lClosure {
	lClosure *parent;
	lClosure *nextFree;
	lTree *data, *meta;
	lBytecodeArray *text;
	lBytecodeOp *ip;
	union {
		lVal *args;
		lVal *exceptionHandler;
	};
	const lSymbol *name;
	lClosure *caller;
	int sp;
	u8 type;
};

struct lThread {
	lVal **valueStack;
	lClosure **closureStack;
	lBytecodeArray *text;
	int valueStackSize;
	int closureStackSize;
	int sp;
	int csp;
};

struct lTree {
	lTree *left;
	lTree *right;
	union {
		const lSymbol *key;
		lTree *nextFree;
	};
	lVal *value;
	i16 height;
	u8 flags;
};
#define TREE_IMMUTABLE 1

struct lNFunc {
	lVal *(*fp)(lClosure *, lVal *);
	lTree *meta;
	lVal *args;
	lSymbol *name;
};

/*
 | Some pretty core Nujel procedures
 */
#define RECURSION_DEPTH_MAX (1<<15)

extern bool    lVerbose;

void      lInit    ();
lClosure *lNewRoot ();
lVal     *lLambda  (lClosure *c, lVal *args, lVal *lambda);
lVal     *lApply   (lClosure *c, lVal *args, lVal *fun);
lClosure *lLoad    (lClosure *c, const char *expr);

int       lListLength (lVal *v);
lVal *lCons(lVal *car, lVal *cdr);

void  lExceptionThrowRaw    (lVal *v) NORETURN;
void  lExceptionThrowValClo (const char *symbol, const char *error, lVal *v, lClosure *c) NORETURN;

static inline lVal *lCar(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.car : NULL;
}

static inline lVal *lCdr(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.cdr : NULL;
}

static inline lVal *lCaar  (lVal *v){return lCar(lCar(v));}
static inline lVal *lCadr  (lVal *v){return lCar(lCdr(v));}
static inline lVal *lCdar  (lVal *v){return lCdr(lCar(v));}
static inline lVal *lCddr  (lVal *v){return lCdr(lCdr(v));}
static inline lVal *lCaddr (lVal *v){return lCar(lCdr(lCdr(v)));}

/*
 | Reader/Printer
 */
lVal *lRead(lClosure *c, const char *str);

char *vspf(char *buf, char *bufEnd, const char *format, va_list va);
char *spf(char *buf, char *bufEnd, const char *format, ...);
void vfpf(FILE *fp, const char *format, va_list va);
void fpf(FILE *f, const char *format, ...);
void epf(const char *format, ...);
void pf(const char *format, ...);

/*
 | Type related procedores
 */
i64             castToInt   (const lVal *v, i64 fallback);
bool            castToBool  (const lVal *v);
const char *    castToString(const lVal *v, const char *fallback);

NORETURN void   throwTypeError          (lClosure *c, lVal *v, lType T);
NORETURN void   throwArityError         (lClosure *c, lVal *v, int arity);
vec             requireVec              (lClosure *c, lVal *v);
vec             requireVecCompatible    (lClosure *c, lVal *v);
i64             requireInt              (lClosure *c, lVal *v);
i64             requireNaturalInt       (lClosure *c, lVal *v);
double          requireFloat            (lClosure *c, lVal *v);
lArray *        requireArray            (lClosure *c, lVal *v);
const lSymbol * requireSymbol           (lClosure *c, lVal *v);
const lSymbol * requireKeyword          (lClosure *c, lVal *v);
const lSymbol * requireSymbolic         (lClosure *c, lVal *v);
lString *       requireString           (lClosure *c, lVal *v);
lTree *         requireTree             (lClosure *c, lVal *v);
lTree *         requireMutableTree      (lClosure *c, lVal *v);
lBytecodeOp     requireBytecodeOp       (lClosure *c, lVal *v);
lBytecodeArray *requireBytecodeArray    (lClosure *c, lVal *v);
lClosure       *requireClosure          (lClosure *c, lVal *v);
lVal           *requireCallable         (lClosure *c, lVal *v);
lVal           *requireEnvironment      (lClosure *c, lVal *v);
lBuffer        *requireBuffer           (lClosure *c, lVal *v);
lBuffer        *requireMutableBuffer    (lClosure *c, lVal *v);
lBufferView    *requireBufferView       (lClosure *c, lVal *v);
lBufferView    *requireMutableBufferView(lClosure *c, lVal *v);

/*
 | Closure related procedores
 */
lVal     *lDefineAliased     (lClosure *c, lVal *lNF, const char *sym);

lVal     *lGetClosureSym     (lClosure *c, const lSymbol *s);
void      lDefineClosureSym  (lClosure *c, const lSymbol *s, lVal *v);
bool      lSetClosureSym     (lClosure *c, const lSymbol *s, lVal *v);
void      lDefineVal         (lClosure *c, const char *str,  lVal *v);

lVal     *lAddNativeFunc     (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lAddNativeFuncFold (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lAddNativeFuncPure (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lAddNativeFuncPureFold (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));

/*
 | Tree related procedures
 */
lTree *lTreeNew             (const lSymbol *s, lVal *v);
lTree *lTreeDup             (const lTree *t);

lVal  *lTreeGet             (const lTree *t, const lSymbol *s, bool *found);
bool   lTreeHas             (const lTree *t, const lSymbol *s, lVal **value);

void   lTreeSet             (      lTree *t, const lSymbol *s, lVal *v, bool *found);
lTree *lTreeInsert          (      lTree *t, const lSymbol *s, lVal *v);

/*
 | lVal related procedures
 */
lVal     *lValSymS      (const lSymbol *s);
lVal     *lValKeywordS  (const lSymbol *s);
lVal     *lValSym       (const char    *s);
lVal     *lValKeyword   (const char    *s);

lVal     *lValBool         (bool v);
lVal     *lValInt          (i64 v);
lVal     *lValFloat        (double v);
lVal     *lValVec          (const vec v);
lVal     *lValTree         (lTree *v);
lVal     *lValObject       (lClosure *v);
lVal     *lValLambda       (lClosure *v);
lVal     *lValString       (const char *s);
lVal     *lValStringLen    (const char *s, int len);
lVal     *lValStringNoCopy (const char *s, int len);
lVal     *lValBufferNoCopy (void *s, size_t len, bool immutable);

extern lSymbol *symNull;
static inline const lSymbol *lGetSymbol(const lVal *v){
	return ((v == NULL) || (v->type != ltSymbol))
		? symNull
		: v->vSymbol;
}

lString  *lStringNew       (const char *str, uint len);
lString  *lStringDup       (      lString *s);
int       lStringLength    (const lString *s);

/*
 | vec related functions
 */
vec   vecNew      (float x, float y, float z, float w);
vec   vecNewP     (const float *p);
vec   vecNOne     ();
vec   vecZero     ();
vec   vecOne      ();
vec   vecInvert   (const vec a);
vec   vecAdd      (const vec a, const vec   b);
vec   vecAddS     (const vec a, const float b);
vec   vecSub      (const vec a, const vec   b);
vec   vecSubS     (const vec a, const float b);
vec   vecMul      (const vec a, const vec   b);
vec   vecMulS     (const vec a, const float b);
vec   vecDiv      (const vec a, const vec   b);
vec   vecDivS     (const vec a, const float b);
vec   vecMod      (const vec a, const vec   b);
vec   vecAbs      (const vec a);
vec   vecFloor    (const vec a);
vec   vecPow      (const vec a, const vec b);
float vecDot      (const vec a, const vec b);
float vecMag      (const vec a);
float vecSum      (const vec a);
float vecAbsSum   (const vec a);
vec   vecSqrt     (const vec a);
vec   vecCross    (const vec a, const vec b);
vec   vecRotate   (const vec a, const vec b, const float rad);
vec   vecNorm     (const vec a);
vec   vecVecToDeg (const vec a);
vec   vecDegToVec (const vec a);
vec   vecCeil     (const vec a);
vec   vecRound    (const vec a);
vec   vecReflect  (const vec i, const vec n);
vec   vecCbrt     (const vec a);


/*
 | GC related procedures
 */
extern void (*rootsMarkerChain)();
extern void (*sweeperChain)();
extern int lGCRuns;


/*
 | Allocator related procedures
 */
lArray *         lArrayAlloc         (size_t len);
lNFunc *         lNFuncAlloc         ();
void             lNFuncFree          (lNFunc *n);
lBytecodeArray * lBytecodeArrayAlloc (size_t len);
lVal *           lValAlloc           (lType t);
int              lBufferViewTypeSize (lBufferViewType T);
lBuffer *        lBufferAlloc        (size_t length, bool immutable);
lBufferView *    lBufferViewAlloc    (lBuffer *buf, lBufferViewType type, size_t offset, size_t length, bool immutable);


/*
 | Symbolic routines
 */
lSymbol  *lSymS         (const char *s);
lSymbol  *lSymSM        (const char *s);
lSymbol  *lSymSL        (const char *s, uint len);

lVal *lnfCat     (lClosure *c, lVal *v);
lVal *lnfArrNew  (lClosure *c, lVal *v);
lVal *lnfTreeNew (lClosure *c, lVal *v);
lVal *lnfVec     (lClosure *c, lVal *v);

#endif
