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
#include <setjmp.h>

#include <stdio.h>
#include <stdlib.h>

/*
 | Core/Exception handling
 */
#define RECURSION_DEPTH_MAX (1<<14)
#define MAX_OPEN_FILE_DESCRIPTORS 256
#define PI    (3.1415926535897932384626433832795)

extern jmp_buf exceptionTarget;
extern lVal    exceptionValue;
extern int     exceptionTargetDepth;

static inline bool isComment(lVal v){
	return v.type == ltComment;
}
static inline lVal lValComment(){
	return lValAlloc(ltComment, NULL);
}
lType lTypecast(const lType a, const lType b);


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
		char c[64];
		struct lSymbol *nextFree;
	};
};

struct lBytecodeArray{
	lBytecodeOp *data;
	lArray *literals;
	union {
		lBytecodeOp *dataEnd;
		struct lBytecodeArray *nextFree;
	};
	u8 flags;
};

typedef enum closureType {
	closureDefault = 0,
	closureObject = 1,
	closureCall = 2,
	closureLet = 3,
	closureTry = 4,
	closureRoot = 5,
} closureType;

struct lNFunc {
	lVal (*fp)(lClosure *, lVal);
	lTree *meta;
	lVal args;
};

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
	lClosure *caller;
	lTree *data, *meta;
	lBytecodeArray *text;
	lBytecodeOp *ip;
	union {
		lVal args;
		lVal exceptionHandler;
	};
	int sp;
	u8 type;
};

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
lClosure *lClosureNewFunCall (lClosure *parent, lVal args, lVal lambda);
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
	lopList            = 0x2E
} lOpcode;

i64   lBytecodeGetOffset16 (const lBytecodeOp *ip);
lVal  lBytecodeEval        (lClosure *c, lBytecodeArray *ops);
lVal  lLambda              (lClosure *c, lVal args, lVal lambda);
lVal  lValBytecodeArray    (const lBytecodeOp *ops, int opsLength, lArray *literals, lClosure *errorClosure);
void  simplePrintVal       (lVal v);
void  simplePrintTree      (lTree *t);

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
lClosure *lRootsClosurePush(lClosure *v);
lSymbol  *lRootsSymbolPush (lSymbol *v);
lThread  *lRootsThreadPush (lThread *v);
extern int rootSP;
extern int lGCRuns;

static inline void lRootsRet(const int i){ rootSP = i; }
static inline int  lRootsGet(){ return rootSP; }

extern bool lGCShouldRunSoon;

void lValGCMark         (lVal v);
void lBufferGCMark      (const lBuffer *v);
void lBufferViewGCMark  (const lBufferView *v);
void lTreeGCMark        (const lTree *v);
void lTreeRootGCMark    (const lTreeRoot *v);
void lClosureGCMark     (const lClosure *c);
void lArrayGCMark       (const lArray *v);
void lNFuncGCMark       (const lNFunc *f);
void lSymbolGCMark      (const lSymbol *v);
void lThreadGCMark      (lThread *c);
void lBytecodeArrayMark (const lBytecodeArray *v);

void lGarbageCollect();
static inline void lGarbageCollectIfNecessary(){
	if(unlikely(lGCShouldRunSoon)){
		lGarbageCollect();
	}
}


/*
 | Alocator related definitions
 */
#define NFN_MAX (1<<10)
#define ARR_MAX (1<<14)
#define CLO_MAX (1<<16)
#define TRE_MAX (1<<19)
#define TRR_MAX (1<<15)
#define BCA_MAX (1<<14)
#define BUF_MAX (1<<15)
#define BFV_MAX (1<<14)
#define CON_MAX (1<<19)

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
#define SYM_MAX (1<<14)
extern lSymbol  lSymbolList [SYM_MAX];
extern lSymbol *lSymbolFFree;
extern uint     lSymbolActive;
extern uint     lSymbolMax;

extern lSymbol *symType;
extern lSymbol *symArguments;
extern lSymbol *symCode;
extern lSymbol *symData;
extern lSymbol *symName;

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

void      lSymbolInit   ();
void      lSymbolFree   (lSymbol *s);
lSymbol  *getTypeSymbol (const lVal a);
lSymbol  *getTypeSymbolT(const lType T);

lBytecodeOp     requireBytecodeOp       (lClosure *c, lVal v);
lBytecodeArray *requireBytecodeArray    (lClosure *c, lVal v);
lClosure       *requireClosure          (lClosure *c, lVal v);
lVal            requireEnvironment      (lClosure *c, lVal v);

lNFunc *         lNFuncAlloc         ();
void             lNFuncFree          (lNFunc *n);
lBytecodeArray * lBytecodeArrayAlloc (size_t len);
lBufferView *    lBufferViewAlloc    (lBuffer *buf, lBufferViewType type, size_t offset, size_t length, bool immutable);
int              lBufferViewTypeSize (lBufferViewType T);

lVal lnfCat     (lClosure *c, lVal v);
lVal lnfArrNew  (lClosure *c, lVal v);
lVal lnfTreeNew (lClosure *c, lVal v);

/*
 | Operations
 */
void lOperationsBase       (lClosure *c);
void lOperationsArithmetic (lClosure *c);
void lOperationsArray      (lClosure *c);
void lOperationsBuffer     (lClosure *c);
void lOperationsBytecode   (lClosure *c);
void lOperationsCore       (lClosure *c);
void lOperationsSpecial    (lClosure *c);
void lOperationsString     (lClosure *c);
void lOperationsTree       (lClosure *c);
void lOperationsGeneric    (lClosure *c);

lVal lAdd(lClosure *c, lVal a, lVal b);
lVal lSub(lClosure *c, lVal a, lVal b);
lVal lMul(lClosure *c, lVal a, lVal b);
lVal lDiv(lClosure *c, lVal a, lVal b);
lVal lRem(lClosure *c, lVal a, lVal b);

lVal lValBytecodeOp(lBytecodeOp v);
lVal lGenericRef(lClosure *c, lVal col, lVal key);

#endif
