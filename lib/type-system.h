#pragma once
#include "nujel.h"

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

void  lOperationsTypeSystem(lClosure *c);

#define lCastIApply(FUNC, c , v) do { \
	if((c == NULL) || (v == NULL)){return lValInt(0);} \
	lVal *t = lCastSpecific(c,v,ltInt); \
	lRootsValPush(t); \
	lVal *d = lValDup(t->vList.car); \
	lRootsValPush(d); \
	lVal *ret = FUNC(d,t); \
	return ret;\
	} while (0)
