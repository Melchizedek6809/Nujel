#pragma once
#include "nujel.h"

i64         castToInt   (const lVal *v, i64         fallback);
double      castToFloat (const lVal *v, double      fallback);
vec         castToVec   (const lVal *v, vec         fallback);
bool        castToBool  (const lVal *v);
const char *castToString(const lVal *v, const char *fallback);
lTree      *castToTree  (const lVal *v, lTree *     fallback);
const lSymbol *castToSymbol(const lVal *v, const lSymbol *fallback);

lVal *lCast             (lClosure *c, lVal *v, lType t);
lVal *lCastAuto         (lClosure *c, lVal *v);
lType lTypecast         (const lType a, const lType b);

void  lOperationsTypeSystem(lClosure *c);

static inline bool isComment(lVal *v){return v && v->type == ltComment;}
