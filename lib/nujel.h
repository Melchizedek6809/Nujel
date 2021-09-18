#pragma once
#include "common.h"

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

typedef struct lClosure lClosure;
typedef struct lVal     lVal;
typedef struct {
	char c[32];
} lSymbol;

typedef struct {
	lVal *car,*cdr;
} lPair;

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

#define VAL_MAX (1<<20)
#define SYM_MAX (1<<14)

#define VAL_MASK ((VAL_MAX)-1)
#define SYM_MASK ((SYM_MAX)-1)

extern lVal     lValList    [VAL_MAX];
extern lSymbol  lSymbolList [SYM_MAX];

extern uint     lValMax;
extern uint     lValActive;

extern lSymbol *symNull,*symQuote,*symArr,*symIf,*symCond,*symWhen,*symUnless,*symLet,*symBegin,*symMinus;

void      lInit             ();
int       lMemUsage         ();
void      lPrintError       (const char *format, ...);

lVal     *lValAlloc         ();
void      lValFree          (lVal *v);

lClosure *lClosureNewRoot   ();

void      lDisplayVal       (lVal *v);
void      lDisplayErrorVal  (lVal *v);
void      lWriteVal         (lVal *v);

lVal     *lApply            (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lCast             (lClosure *c, lVal *v, lType t);
lVal     *lEval             (lClosure *c, lVal *v);
lType     lTypecast         (const lType a,const lType b);
lType     lTypecastList     (lVal *a);

lVal     *lValBool          (bool v);
lVal     *lValInf           ();
lVal     *lValInt           (int v);
lVal     *lValFloat         (float v);
lVal     *lValSymS          (const lSymbol *s);
lVal     *lValSym           (const char *s);
lSymbol  *lSymS             (const char *s);
lSymbol  *lSymSL            (const char *s, uint len);
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

int       lSymCmp           (const lVal *a,const lVal *b);
int       lSymEq            (const lSymbol *a,const lSymbol *b);

#define forEach(n,v) for(lVal *n = v;(n != NULL) && (n->type == ltPair) && (n->vList.car != NULL); n = n->vList.cdr)

#define lValD(i) (i == 0 ? NULL : &lValList[i & VAL_MASK])
#define lValI(v) (v == NULL ? 0 : v - lValList)

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
