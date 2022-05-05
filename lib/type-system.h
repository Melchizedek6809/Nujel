#ifndef NUJEL_LIB_TYPE_SYSTEM
#define NUJEL_LIB_TYPE_SYSTEM

#include "nujel.h"
#include "misc/vec.h"

i64            castToInt   (const lVal *v, i64 fallback);
double         castToFloat (const lVal *v, double fallback);
vec            castToVec   (const lVal *v, vec fallback);
bool           castToBool  (const lVal *v);
const char *   castToString(const lVal *v, const char *fallback);
lTree *        castToTree  (const lVal *v, lTree *fallback);
const lSymbol *castToSymbol(const lVal *v, const lSymbol *fallback);

vec            requireVec          (lClosure *c, lVal *v);
i64            requireInt          (lClosure *c, lVal *v);
i64            requireNaturalInt   (lClosure *c, lVal *v);
double         requireFloat        (lClosure *c, lVal *v);
lArray *       requireArray        (lClosure *c, lVal *v);
const lSymbol *requireSymbol       (lClosure *c, lVal *v);
const lSymbol *requireSymbolic     (lClosure *c, lVal *v);
lString *      requireString       (lClosure *c, lVal *v);
lTree *        requireTree         (lClosure *c, lVal *v);
lTree *        requireMutableTree  (lClosure *c, lVal *v);
lBytecodeOp    requireBytecodeOp   (lClosure *c, lVal *v);
lBytecodeArray requireBytecodeArray(lClosure *c, lVal *v);

lVal *lCast             (lClosure *c, lVal *v, lType t);
lVal *lCastAuto         (lClosure *c, lVal *v);
lType lTypecast         (const lType a, const lType b);

void  lOperationsTypeSystem(lClosure *c);

static inline bool isComment(lVal *v){return v && v->type == ltComment;}


#endif
