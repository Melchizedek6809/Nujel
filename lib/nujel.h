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
typedef struct lSymbol  lSymbol;
typedef struct lVal     lVal;

typedef struct {
	lVal *car,*cdr;
} lPair;
#define lfSpecial   (16)

void      lInit             ();
int       lMemUsage         ();
void      lPrintError       (const char *format, ...);

lClosure *lClosureNewRoot   ();
lClosure *lClosureNewRootNoStdLib();

void      lDisplayVal       (lVal *v);
void      lDisplayErrorVal  (lVal *v);
void      lWriteVal         (lVal *v);

lVal     *lApply            (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lCast             (lClosure *c, lVal *v, lType t);
lVal     *lEval             (lClosure *c, lVal *v);
lType     lTypecast         (const lType a,const lType b);
lType     lTypecastList     (lVal *a);
lType     lGetType          (lVal *v);

lVal     *getLArgB          (lClosure *c, lVal *v, bool *res);
lVal     *getLArgI          (lClosure *c, lVal *v, int *res);
lVal     *getLArgF          (lClosure *c, lVal *v, float *res);
lVal     *getLArgV          (lClosure *c, lVal *v, vec *res);
lVal     *getLArgL          (lClosure *c, lVal *v, lVal **res);
lVal     *getLArgS          (lClosure *c, lVal *v, const char **res);
lVal     *lConst            (lVal *v);
lVal     *lnfBegin          (lClosure *c, lVal *v);
lVal     *lWrap             (lVal *v);
lVal     *lEvalCast         (lClosure *c, lVal *v);
lVal     *lEvalCastSpecific (lClosure *c, lVal *v, const lType type);
lVal     *lEvalCastNumeric  (lClosure *c, lVal *v);

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
