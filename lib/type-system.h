#pragma once
#include "nujel.h"

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
	ltTree,

	ltLambda,
	ltDynamic,
	ltObject,
	ltNativeFunc,
	ltSpecialForm,

	ltGUIWidget
} lType;

int         castToInt   (const lVal *v, int         fallback);
float       castToFloat (const lVal *v, float       fallback);
vec         castToVec   (const lVal *v, vec         fallback);
bool        castToBool  (const lVal *v);
const char *castToString(const lVal *v, const char *fallback);
lTree      *castToTree  (const lVal *v, lTree *     fallback);

lVal *lCast             (lClosure *c, lVal *v, lType t);
lVal *lCastAuto         (lClosure *c, lVal *v);
lVal *lCastSpecific     (lClosure *c, lVal *v, const lType type);
lVal *lCastNumeric      (lClosure *c, lVal *v);
lType lTypecast         (const lType a, const lType b);
lType lTypecastList     (lVal *a);

void  lOperationsTypeSystem(lClosure *c);

#define lCastIApply(FUNC, c , v) do { \
	if((c == NULL) || (v == NULL)){return lValInt(0);} \
	lVal *t = lCastSpecific(c,v,ltInt); \
	lRootsValPush(t); \
	lVal *d = lValDup(t->vList.car); \
	lRootsValPush(d); \
	lVal *ret = FUNC(d,t); \
	lRootsValPop(); \
	lRootsValPop(); \
	return ret;\
	} while (0)

#define lCastApply(FUNC, c , v) do { \
	lVal *t = lCastAuto(c,v); \
	if(t == NULL){return NULL;} \
	lRootsValPush(t); \
	lVal *d = lValDup(t->vList.car); \
	if(d == NULL){lRootsValPop(); return NULL;} \
	lRootsValPush(d); \
	lVal *ret; \
	switch(d->type){ \
	default:      ret = lValInt(0);   break; \
	case ltInf:   ret = lValInf();    break; \
	case ltInt:   ret = FUNC##I(d,t); break; \
	case ltFloat: ret = FUNC##F(d,t); break; \
	case ltVec:   ret = FUNC##V(d,t); break; \
	} \
	lRootsValPop(); \
	lRootsValPop(); \
	return ret; \
	} while (0)
