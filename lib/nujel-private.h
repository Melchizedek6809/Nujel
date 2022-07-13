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

#include "../nujel.h"
#include <setjmp.h>


/*
 | Core/Exception handling
 */
extern jmp_buf exceptionTarget;
extern lVal   *exceptionValue;
extern int     exceptionTargetDepth;

static inline bool isComment(lVal *v){return v && v->type == ltComment;}
static inline lVal *lValComment(){return lValAlloc(ltComment);}
lType lTypecast(const lType a, const lType b);


/*
 | Tree related procedures
 */
uint   lTreeSize            (const lTree *t);
lVal  *lTreeToList          (const lTree *t);
lVal  *lTreeKeysToList      (const lTree *t);
lVal  *lTreeValuesToList    (const lTree *t);


/*
 | Closure related procedures
 */
lClosure *lClosureNew        (lClosure *parent, closureType t);
lClosure *lClosureNewFunCall (lClosure *parent, lVal *args, lVal *lambda);
void      lClosureSetMeta    (lClosure *c, lVal *doc);
bool      lHasClosureSym     (lClosure *c, const lSymbol *s, lVal **v);
lVal     *lLambdaNew         (lClosure *parent, lVal *name, lVal *args, lVal *body);


/*
 | lVal related procedures
 */
int       lValCompare      (const lVal *a, const lVal *b);
bool      lValEqual        (const lVal *a, const lVal *b);
i64       lValGreater      (const lVal *a, const lVal *b);
lVal     *lValStringError  (const char *bufStart, const char *bufEnd, const char *errStart, const char *err, const char *errEnd);


/*
 | Bytecode related definitions
 */
typedef enum lOpcode {
	lopNOP             =  0x0,
	lopRet             =  0x1,
	lopIntByte         =  0x2,
	lopIntAdd          =  0x3,
	lopApply           =  0x4,
	lopGet             =  0x5,
	lopPushValExt      =  0x6,
	lopDef             =  0x7,
	lopSet             =  0x8,
	lopJmp             =  0x9,
	lopJt              =  0xA,
	lopJf              =  0xB,
	lopDup             =  0xC,
	lopDrop            =  0xD,
	lopUNUSEDX0E       =  0xE,
	lopUNUSEDX0F       =  0xF,
	lopUNUSEDX10       = 0x10,
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
	lopUNUSEDX1D       = 0x1D,
	lopLessPred        = 0x1E,
	lopLessEqPred      = 0x1F,
	lopEqualPred       = 0x20,
	lopGreaterEqPred   = 0x21,
	lopGreaterPred     = 0x22,
	lopUNUSED23        = 0x23,
	lopPushNil         = 0x24,
	lopAdd             = 0x25,
	lopSub             = 0x26,
	lopMul             = 0x27,
	lopDiv             = 0x28,
	lopRem             = 0x29,
	lopZeroPred        = 0x2A
} lOpcode;

const char *lBytecodeGetOpcodeName(const lBytecodeOp op);
lBytecodeOp *lBytecodeReadOPSym(lBytecodeOp *ip, lSymbol **ret);
int lBytecodeGetOffset16(const lBytecodeOp *ip);

lVal *lBytecodeEval(lClosure *c, lBytecodeArray *ops, bool trace);
void lBytecodeTrace(const lThread *ctx, lBytecodeOp *ip, const lBytecodeArray *ops);


/*
 | Workarounds for missing builtins
 */
#if defined(__TINYC__) || defined(__WATCOMC__)
uint32_t __builtin_popcount(uint32_t x);
uint64_t __builtin_popcountll(uint64_t x);
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
lVal     *lRootsValPush    (lVal *c);
lClosure *lRootsClosurePush(lClosure *v);
lSymbol  *lRootsSymbolPush (lSymbol *v);
lThread  *lRootsThreadPush (lThread *v);
void      lRootsMark       ();

extern int rootSP;

static inline void lRootsRet(const int i){ rootSP = i; }
static inline int  lRootsGet(){ return rootSP; }

extern bool lGCShouldRunSoon;

void lWidgetMarkI       (uint i);

void lValGCMark         (lVal *v);
void lBufferGCMark      (const lBuffer *v);
void lBufferViewGCMark  (const lBufferView *v);
void lTreeGCMark        (const lTree *v);
void lClosureGCMark     (const lClosure *c);
void lArrayGCMark       (const lArray *v);
void lNFuncGCMark       (const lNFunc *f);
void lSymbolGCMark      (const lSymbol *v);
void lThreadGCMark      (lThread *c);
void lBytecodeArrayMark (const lBytecodeArray *v);

void lGarbageCollect();
static inline void lGarbageCollectIfNecessary(){
	if(lGCShouldRunSoon){
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
#define VAL_MAX (1<<21)
#define BCA_MAX (1<<14)
#define BUF_MAX (1<<15)
#define BFV_MAX (1<<14)

#define allocatorTypes() \
	defineAllocator(lArray, ARR_MAX) \
	defineAllocator(lClosure, CLO_MAX) \
	defineAllocator(lTree, TRE_MAX) \
	defineAllocator(lVal, VAL_MAX) \
	defineAllocator(lBytecodeArray, BCA_MAX) \
	defineAllocator(lBuffer, BUF_MAX) \
	defineAllocator(lBufferView, BFV_MAX)

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
static inline int lValIndex(const lVal *v){
	return v - lValList;
}
static inline lVal *lIndexVal(uint i){
	return (i >= lValMax) ? NULL : &lValList[i];
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
lSymbol  *getTypeSymbol (const lVal *a);
lSymbol  *getTypeSymbolT(const lType T);

static inline int lSymIndex(const lSymbol *s){
	return s - lSymbolList;
}
static inline lSymbol *lIndexSym(uint i){
	return (i >= lSymbolMax) ? NULL : &lSymbolList[i];
}


/*
 | Some operations
 */
void lOperationsBase(lClosure *c);

lVal *lAdd(lClosure *c, lVal *a, lVal *b);
lVal *lSub(lClosure *c, lVal *a, lVal *b);
lVal *lMul(lClosure *c, lVal *a, lVal *b);
lVal *lDiv(lClosure *c, lVal *a, lVal *b);
lVal *lRem(lClosure *c, lVal *a, lVal *b);

#endif
