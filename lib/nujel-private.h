/*
 | nujel-private.h
 |
 | This file contains all the internal definitions needed for the Nujel runtime.
 | No external program should include this file, since it WILL change a lot and
 | a lot of the functions aren't very easy to use. To interface to Nujel one
 | should use nujel-public.h instead.
 */
#ifndef NUJEL_LIB_NUJEL_PRIVATE
#define NUJEL_LIB_NUJEL_PRIVATE

#ifndef NUJEL_AMALGAMATION
#include "nujel.h"
#endif

#include <stdio.h>
#include <stdlib.h>

/*
 | Core
 */
#define MAX_OPEN_FILE_DESCRIPTORS 256
#define PI    (3.1415926535897932384626433832795)

static inline bool isComment(lVal v){
	return v.type == ltComment;
}
static inline lVal lValComment(){
	return lValAlloc(ltComment, NULL);
}
lType lTypecast(const lType a, const lType b);

struct lClass {
	const lSymbol *name;
	lClass *parent;
	lTree *methods;
	lTree *staticMethods;
};
extern lClass lClassList[64];

struct lArray {
	lVal *data;
	union {
		lArray *nextFree;
		struct {
			i32 length;
			u8 flags;
		};
	};
};
#define ARRAY_IMMUTABLE 1

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

struct lSymbol {
	union {
		char c[94];
		struct lSymbol *nextFree;
	};
};

struct lBytecodeArray {
	lBytecodeOp *data;
	lArray *literals;
	union {
		lBytecodeOp *dataEnd;
		struct lBytecodeArray *nextFree;
	};
};

struct lNFunc {
	union {
		lVal (*fp)();
		lVal (*fpC)(lClosure *);
		lVal (*fpV)(lVal);
		lVal (*fpCV)(lClosure *, lVal);
		lVal (*fpVV)(lVal, lVal);
		lVal (*fpCVV)(lClosure *, lVal, lVal);
		lVal (*fpVVV)(lVal, lVal, lVal);
		lVal (*fpCVVV)(lClosure *, lVal, lVal, lVal);
		lVal (*fpVVVV)(lVal, lVal, lVal, lVal);
		lVal (*fpCVVVV)(lClosure *, lVal, lVal, lVal, lVal);
		lVal (*fpVVVVV)(lVal, lVal, lVal, lVal, lVal);
		lVal (*fpCVVVVV)(lClosure *, lVal, lVal, lVal, lVal, lVal);
		lVal (*fpVVVVVV)(lVal, lVal, lVal, lVal, lVal, lVal);
		lVal (*fpCVVVVVV)(lClosure *, lVal, lVal ,lVal, lVal, lVal, lVal);
		lVal (*fpVVVVVVV)(lVal, lVal, lVal ,lVal, lVal, lVal, lVal);
		lVal (*fpCVVVVVVV)(lClosure *, lVal, lVal, lVal ,lVal, lVal, lVal, lVal);
		lVal (*fpR)(lVal);
		lVal (*fpCR)(lClosure *, lVal);
	};
	lTree *meta;
	lVal args;
	u8 argCount;
};

#define NFUNC_FOLD 1
#define NFUNC_PURE 2

struct lTree {
	lTree *left;
	lTree *right;
	union {
		const lSymbol *key;
		lTree *nextFree;
	};
	lVal value;
	i16 height;
	u8 flags;
};
#define TREE_IMMUTABLE 1

struct lClosure {
	union {
		lClosure *parent;
		lClosure *nextFree;
	};
	lTree *data, *meta;
	lBytecodeArray *text;
	const lBytecodeOp *ip;
	union {
		lVal args;
		lVal exceptionHandler;
	};
	u16 sp;
	u8 type;
};

typedef enum closureType {
	closureDefault = 0,
	closureObject = 1,
	closureCall = 2,
	closureLet = 3,
	closureTry = 4,
	closureRoot = 5,
} closureType;


struct lThread {
	lBytecodeArray *text;

	lVal *valueStack;
	lClosure **closureStack;

	int sp;
	int valueStackSize;

	int csp;
	int closureStackSize;
};

/*
 | Closure related procedures
 */
lClosure *lClosureNew        (lClosure *parent, closureType t);
lClosure *lClosureNewFunCall (lVal args, lVal lambda);
void      lClosureSetMeta    (lClosure *c, lVal doc);
bool      lHasClosureSym     (lClosure *c, const lSymbol *s, lVal *v);
lVal      lLambdaNew         (lClosure *parent, lVal args, lVal body);

/*
 | lVal related procedures
 */
int       lValCompare      (const lVal a, const lVal b);
bool      lValEqual        (const lVal a, const lVal b);
i64       lValGreater      (const lVal a, const lVal b);
lVal      lValStringError  (const char *bufStart, const char *bufEnd, const char *errStart, const char *err, const char *errEnd);

/*
 | Bytecode related definitions
 */
typedef enum lOpcode {
	lopNOP             =  0x0,
	lopRet             =  0x1,
	lopIntByte         =  0x2,
	lopIntAdd          =  0x3,
	lopApply           =  0x4,
	lopSetVal          =  0x5,
	lopPushValExt      =  0x6,
	lopDefVal          =  0x7,
	lopDefValExt       =  0x8,
	lopJmp             =  0x9,
	lopJt              =  0xA,
	lopJf              =  0xB,
	lopDup             =  0xC,
	lopDrop            =  0xD,
	lopGetVal          =  0xE,
	lopGetValExt       =  0xF,
	lopSetValExt       = 0x10,
	lopCar             = 0x11,
	lopCdr             = 0x12,
	lopClosurePush     = 0x13,
	lopCons            = 0x14,
	lopLet             = 0x15,
	lopClosurePop      = 0x16,
	lopFnDynamic       = 0x17,
	lopMacroDynamic    = 0x18,
	lopTry             = 0x19,
	lopPushVal         = 0x1A,
	lopPushTrue        = 0x1B,
	lopPushFalse       = 0x1C,
	lopEval            = 0x1D,
	lopLessPred        = 0x1E,
	lopLessEqPred      = 0x1F,
	lopEqualPred       = 0x20,
	lopGreaterEqPred   = 0x21,
	lopGreaterPred     = 0x22,
	lopIncInt          = 0x23,
	lopPushNil         = 0x24,
	lopAdd             = 0x25,
	lopSub             = 0x26,
	lopMul             = 0x27,
	lopDiv             = 0x28,
	lopRem             = 0x29,
	lopZeroPred        = 0x2A,
	lopRef             = 0x2B,
	lopCadr            = 0x2C,
	lopMutableEval     = 0x2D,
	lopList            = 0x2E,
	lopThrow           = 0x2F,
	lopApplyCollection = 0x30,
	lopBitShiftLeft    = 0x31,
	lopBitShiftRight   = 0x32,
	lopBitAnd          = 0x33,
	lopBitOr           = 0x34,
	lopBitXor          = 0x35,
	lopBitNot          = 0x36,
	lopGenSet          = 0x37,
} lOpcode;

lVal  lBytecodeEval        (lClosure *c, lBytecodeArray *ops);
lVal  lValBytecodeArray    (const lBytecodeOp *ops, int opsLength, lArray *literals);
void  simplePrintVal       (lVal v);

/*
 | Workarounds for missing builtins
 */
#if false
uint32_t __builtin_popcount(uint32_t x);
uint64_t __builtin_popcountll(uint64_t x);
#endif

#if defined(__TINYC__)
void __sync_synchronize();
#endif


/*
 | Compatibility procedures
 */
u64 getMSecs();
void lAddPlatformVars(lClosure *c);

/*
 | GC related procedures
 */
lSymbol  *lRootsSymbolPush (lSymbol *v);
lThread  *lRootsThreadPush (lThread *v);
extern int rootSP;
extern int lGCRuns;

static inline void lRootsRet(const int i){ rootSP = i; }
static inline int  lRootsGet(){ return rootSP; }

extern bool lGCShouldRunSoon;

void lGarbageCollect();

/*
 | Alocator related definitions
 */
#define SYM_MAX (1<<14)
#define NFN_MAX (1<<10)
#define ARR_MAX (1<<14)
#define CLO_MAX (1<<15)
#define TRR_MAX (1<<15)
#define BCA_MAX (1<<14)
#define BUF_MAX (1<<15)
#define BFV_MAX (1<<14)
#define TRE_MAX (1<<17)
#define CON_MAX (1<<18)

#define allocatorTypes() \
	defineAllocator(lArray, ARR_MAX) \
	defineAllocator(lClosure, CLO_MAX) \
	defineAllocator(lTree, TRE_MAX) \
	defineAllocator(lTreeRoot, TRR_MAX) \
	defineAllocator(lBytecodeArray, BCA_MAX) \
	defineAllocator(lBuffer, BUF_MAX) \
	defineAllocator(lBufferView, BFV_MAX) \
	defineAllocator(lPair, CON_MAX)

#define defineAllocator(T, typeMax) \
extern T T##List[typeMax]; \
extern uint T##Max;	   \
extern uint T##Active; \
extern T * T##FFree; \
T * T##AllocRaw(); \
void T##Free(T * v);

allocatorTypes()
#undef defineAllocator

extern lNFunc   lNFuncList[NFN_MAX];
extern uint     lNFuncMax;


static inline int lClosureID(const lClosure *n){
	return n - lClosureList;
}
static inline int lNFuncID(const lNFunc *n){
	return n - lNFuncList;
}

/*
 | Symbolic procedures
 */
extern lSymbol  lSymbolList [SYM_MAX];
extern lSymbol *lSymbolFFree;
extern uint     lSymbolActive;
extern uint     lSymbolMax;

extern lSymbol *symType;
extern lSymbol *symArguments;
extern lSymbol *symCode;
extern lSymbol *symData;
extern lSymbol *symName;

extern lSymbol *lSymLTNil;
extern lSymbol *lSymLTBool;
extern lSymbol *lSymLTPair;
extern lSymbol *lSymLTLambda;
extern lSymbol *lSymLTInt;
extern lSymbol *lSymLTFloat;
extern lSymbol *lSymLTString;
extern lSymbol *lSymLTSymbol;
extern lSymbol *lSymLTKeyword;
extern lSymbol *lSymLTNativeFunction;
extern lSymbol *lSymLTEnvironment;
extern lSymbol *lSymLTMacro;
extern lSymbol *lSymLTArray;
extern lSymbol *lSymLTTree;
extern lSymbol *lSymLTBytecodeArray;
extern lSymbol *lSymLTBuffer;
extern lSymbol *lSymLTBufferView;
extern lSymbol *lSymLTFileHandle;
extern lSymbol *lSymLTUnknownType;
extern lSymbol *lSymLTType;

extern lSymbol *symNull;
extern lSymbol *symQuote;
extern lSymbol *symQuasiquote;
extern lSymbol *symUnquote;
extern lSymbol *symUnquoteSplicing;
extern lSymbol *symArr;
extern lSymbol *symTreeNew;
extern lSymbol *symDocumentation;
extern lSymbol *symPure;
extern lSymbol *symFold;

extern lSymbol *lSymVMError;


void      lSymbolInit   ();
void      lSymbolFree   (lSymbol *s);

lSymbol  *getTypeSymbol (const lVal a);
lSymbol  *getTypeSymbolT(const lType T);

lNFunc *         lNFuncAlloc         ();
void             lNFuncFree          (lNFunc *n);
lBytecodeArray * lBytecodeArrayAlloc (size_t len);
lBufferView *    lBufferViewAlloc    (lBuffer *buf, lBufferViewType type, size_t offset, size_t length, bool immutable);
int              lBufferViewTypeSize (lBufferViewType T);

lVal lnfArrNew  (lVal v);
lVal lnfTreeNew (lVal v);

/*
 | Operations
 */
void lOperationsBase       (lClosure *c);
void lOperationsArithmetic (lClosure *c);
void lOperationsArray      (lClosure *c);
void lOperationsBuffer     (lClosure *c);
void lOperationsCore       (lClosure *c);
void lOperationsSpecial    (lClosure *c);
void lOperationsTree       (lClosure *c);
void lOperationsGeneric    (lClosure *c);
void lOperationsString     ();
void lOperationsBytecode   ();

lVal lValBytecodeOp(lBytecodeOp v);
lVal lGenericRef(lVal col, lVal key);
lVal lGenericSet(lVal col, lVal key, lVal v);

void lTypesInit();
lVal lMethodLookup(const lSymbol *method, lVal self);

#endif
