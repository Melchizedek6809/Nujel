#pragma once
#include "common.h"

typedef enum lType {
	ltNoAlloc = 0,

	ltSymbol,
	ltBool,
	ltInt,
	ltFloat,
	ltVec,
	ltInf,

	ltPair,
	ltString,
	ltArray,

	ltLambda,
	ltNativeFunc,
	ltSpecialForm,

	ltGUIWidget
} lType;

typedef struct lArray   lArray;;
typedef struct lClosure lClosure;
typedef struct lNFunc   lNFunc;;
typedef struct lSymbol  lSymbol;
typedef struct lString  lString;;
typedef struct lVec     lVec;
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

lVal     *lMap              (lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *));
lVal     *lEval             (lClosure *c, lVal *v);

lVal     *lnfBegin          (lClosure *c, lVal *v);
lVal     *lWrap             (lVal *v);

#define lCastIApply(FUNC, c , v) do { \
	if((c == NULL) || (v == NULL)){return lValInt(0);} \
	lVal *t = lCastSpecific(c,v,ltInt); \
	if((t == NULL) || (t->type != ltPair)){return lValInt(0);} \
	lVal *d = lValDup(t->vList.car); \
	if(d == NULL){return lValInt(0);} \
	return FUNC(d,t); \
	} while (0)

#define lCastApply(FUNC, c , v) do { \
	lVal *t = lCastAuto(c,v); \
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
