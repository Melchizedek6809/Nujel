/*
 * Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This is the main external header file to use when integrating a Nujel runtime
 * into your project.
 */
#pragma once
#include "common.h"

extern int lGCRuns;

typedef enum lType {
	ltNoAlloc = 0,
	ltBool,
	ltPair,
	ltLambda,
	ltInt,
	ltFloat,
	ltVec,
	ltString,
	ltSymbol,
	ltNativeFunc,
	ltSpecialForm,
	ltInf,
	ltArray,
	ltGUIWidget
} lType;

typedef struct lVal     lVal;
typedef struct lClosure lClosure;
typedef struct lString  lString;
typedef struct lArray   lArray;
typedef struct lNFunc   lNFunc;
typedef struct {
	char c[32];
} lSymbol;

typedef struct {
	lVal *car,*cdr;
} lPair;

typedef struct {
	vec v;
	u16 nextFree;
	u16 flags;
} lVec;

struct lArray {
	u32 *data;
	i32 length;
	u16 flags;
	u16 nextFree;
};

struct lNFunc {
	lVal *(*fp)(lClosure *, lVal *);
	lVal *doc;
	u16 flags;
	u16 nextFree;
};
#define lfSpecial   (16)

struct lVal {
	u8 flags;
	u8 type;
	union {
		u32        vCdr;
		bool       vBool;
		lPair      vList;
		int        vInt;
		float      vFloat;
	};
};
#define lfMarked    ( 1)
#define lfNoGC      ( 2)
#define lfConst     ( 4)
#define lfInUse     ( 8)

struct lClosure {
	lVal *data;
	lVal *text;
	u16 parent;
	u16 nextFree;
	u16 flags;
	u16 refCount;
};
#define lfDynamic   (16)
#define lfObject    (32)
#define lfUsed      (64)

struct lString {
	const char *buf,*data,*bufEnd;
	u16 nextFree;
	u16 flags;
};
#define lfHeapAlloc (16)

#define VAL_MAX (1<<20)
#define CLO_MAX (1<<16)
#define STR_MAX (1<<14)
#define ARR_MAX (1<<12)
#define NFN_MAX (1<<10)
#define VEC_MAX (1<<14)
#define SYM_MAX (1<<14)

#define VAL_MASK ((VAL_MAX)-1)
#define CLO_MASK ((CLO_MAX)-1)
#define STR_MASK ((STR_MAX)-1)
#define ARR_MASK ((ARR_MAX)-1)
#define NFN_MASK ((NFN_MAX)-1)
#define VEC_MASK ((VEC_MAX)-1)
#define SYM_MASK ((SYM_MAX)-1)

extern lVal     lValList    [VAL_MAX];
extern lClosure lClosureList[CLO_MAX];
extern lString  lStringList [STR_MAX];
extern lArray   lArrayList  [ARR_MAX];
extern lNFunc   lNFuncList  [NFN_MAX];
extern lVec     lVecList    [VEC_MAX];
extern lSymbol  lSymbolList [SYM_MAX];

extern lSymbol *symNull,*symQuote,*symArr,*symIf,*symCond,*symWhen,*symUnless,*symLet,*symBegin,*symIntAt,*symFloatAt,*symVecAt, *symMinus;

void      lInit             ();
int       lMemUsage         ();
void      lPrintError       (const char *format, ...);

uint      lClosureAlloc     ();
lClosure *lClosureNewRoot   ();
uint      lClosureNew       (uint parent);
void      lClosureFree      (uint c);
lVal     *lClosureAddNF     (uint c, const char *sym, lVal *(*func)(lClosure *,lVal *));

lVal     *lValAlloc         ();
void      lValFree          (lVal *v);

uint      lArrayAlloc       ();
void      lArrayFree        (uint v);

uint      lStringAlloc      ();
void      lStringFree       (uint v);
uint      lStringNew        (const char *str, uint len);
uint      lStringDup        (uint i);

void      lClosureGC        ();
void      lDisplayVal       (lVal *v);
void      lDisplayErrorVal  (lVal *v);
void      lWriteVal         (lVal *v);

void      lDefineVal        (lClosure *c, const char *sym, lVal *val);
lVal     *lAddNativeFunc    (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lValNativeFunc    (lVal *(*func)(lClosure *,lVal *), lVal *args, lVal *docString);
lVal     *lGetClosureSym    (uint      c, lSymbol *s);
lVal     *lResolveClosureSym(uint      c, lSymbol *s);
lVal     *lDefineClosureSym (uint      c, lSymbol *s);
lVal     *lMatchClosureSym  (uint      c, lVal *v, lSymbol *s);
lVal     *lSearchClosureSym (uint      c, lVal *v, const char *str, uint len);
lVal     *lResolveSym       (uint      c, lVal *v);
lVal     *lApply            (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lCast             (lClosure *c, lVal *v, lType t);
lVal     *lEval             (lClosure *c, lVal *v);
lType     lTypecast         (const lType a,const lType b);
lType     lTypecastList     (lVal *a);

lVal     *lValBool          (bool v);
lVal     *lValInf           ();
lVal     *lValInt           (int v);
lVal     *lValFloat         (float v);
lVal     *lValVec           (const vec v);
lVal     *lValSymS          (const lSymbol *s);
lVal     *lValSym           (const char *s);
lSymbol  *lSymS             (const char *s);
lSymbol  *lSymSL            (const char *s, uint len);
lVal     *lValString        (const char *s);
lVal     *lnfCat            (lClosure *c, lVal *v);
lVal     *lValCopy          (lVal *dst, const lVal *src);
lVal     *getLArgB          (lClosure *c, lVal *v, bool *res);
lVal     *getLArgI          (lClosure *c, lVal *v, int *res);
lVal     *getLArgF          (lClosure *c, lVal *v, float *res);
lVal     *getLArgV          (lClosure *c, lVal *v, vec *res);
lVal     *getLArgL          (lClosure *c, lVal *v, lVal **res);
lVal     *getLArgS          (lClosure *c, lVal *v, const char **res);
lVal     *lConst            (lVal *v);
lVal     *lValDup           (const lVal *v);
lVal     *lnfBegin          (lClosure *c, lVal *v);
lVal     *lWrap             (lVal *v);
lVal     *lEvalCast         (lClosure *c, lVal *v);
lVal     *lEvalCastSpecific (lClosure *c, lVal *v, const lType type);
lVal     *lEvalCastNumeric  (lClosure *c, lVal *v);

lVal     *lCons             (lVal *car,lVal *cdr);
lVal     *lCar              (lVal *v);
lVal     *lCdr              (lVal *v);
lVal     *lCaar             (lVal *v);
lVal     *lCadr             (lVal *v);
lVal     *lCdar             (lVal *v);
lVal     *lCddr             (lVal *v);
lVal     *lCadar            (lVal *v);
lVal     *lCaddr            (lVal *v);
lVal     *lCdddr            (lVal *v);
lVal     *lLastCar          (lVal *v);
int       lListLength       (lVal *v);
lType     lGetType          (lVal *v);

int       lStringLength     (const lString *s);
int       lSymCmp           (const lVal *a,const lVal *b);
int       lSymEq            (const lSymbol *a,const lSymbol *b);

#define forEach(n,v) for(lVal *n = v;(n != NULL) && (n->type == ltPair) && (n->vList.car != NULL); n = n->vList.cdr)

#define lVec(i)      lVecList[i & VEC_MASK]
#define lVecV(i)     lVec(i).v
#define lVecFlags(i) lVec(i).flags

#define lStrNull(val)  (((val->vCdr & STR_MASK) == 0) || (lStringList[val->vCdr & STR_MASK].data == NULL))
#define lStr(val)      lStringList[val->vCdr & STR_MASK]
#define lStrData(val)  lStr(val).data
#define lStrBuf(val)   lStr(val).buf
#define lStrEnd(val)   lStr(val).bufEnd
#define lStrFlags(val) lStr(val).flags

#define lArrNull(val)   (((val->vCdr & ARR_MASK) == 0) || (lArrayList[val->vCdr & ARR_MASK].data == NULL))
#define lArr(val)       lArrayList[val->vCdr & ARR_MASK]
#define lArrLength(val) lArr(val).length
#define lArrData(val)   lArr(val).data

#define lClo(i)       lClosureList[i & CLO_MASK]
#define lCloI(c)      (c == NULL ? 0 : c - lClosureList)
#define lCloParent(i) lClosureList[i & CLO_MASK].parent
#define lCloData(i)   lClosureList[i & CLO_MASK].data
#define lCloText(i)   lClosureList[i & CLO_MASK].text

#define lValD(i) (i == 0 ? NULL : &lValList[i & VAL_MASK])
#define lValI(v) (v == NULL ? 0 : v - lValList)

#define lNFN(i) lNFuncList[i & NFN_MASK]
#define lvSym(i)  (i == 0 ? symNull : &lSymbolList[i & SYM_MASK])
#define lvSymI(s) (s == NULL ? 0 : s - lSymbolList)
#define lGetSymbol(v) (((v == NULL) || (v->type != ltSymbol)) ? symNull : lvSym(v->vCdr))

#define lEvalCastIApply(FUNC, c , v) do { \
	if((c == NULL) || (v == NULL)){return lValInt(0);} \
	lVal *t = lEvalCastSpecific(c,v,ltInt); \
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);} \
	lVal *d = lValDup(t->vList.car); \
	if(d == NULL){return lValInt(0);} \
	return FUNC(d,t); \
	} while (0)

#define lEvalCastApply(FUNC, c , v) do { \
	lVal *t = lEvalCast(c,v); \
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);} \
	lVal *d = lValDup(t->vList.car); \
	if(d == NULL){return lValInt(0);} \
	switch(d->type){ \
	default:      return lValInt(0); \
	case ltInf:   return lValInf(); \
	case ltInt:   return FUNC##I(d,t); \
	case ltFloat: return FUNC##F(d,t); \
	case ltVec:   return FUNC##V(d,t); \
	}} while (0)
