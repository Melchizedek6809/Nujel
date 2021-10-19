#pragma once
#include "nujel.h"

int         castToInt   (const lVal *v, int         fallback);
float       castToFloat (const lVal *v, float       fallback);
vec         castToVec   (const lVal *v, vec         fallback);
bool        castToBool  (const lVal *v);
const char *castToString(const lVal *v, const char *fallback);

lVal *lCast             (lClosure *c, lVal *v, lType t);
lVal *lCastAuto         (lClosure *c, lVal *v);
lVal *lCastSpecific     (lClosure *c, lVal *v, const lType type);
lVal *lCastNumeric      (lClosure *c, lVal *v);
lType lTypecast         (const lType a, const lType b);
lType lTypecastList     (lVal *a);

void  lOperationsTypeSystem(lClosure *c);
