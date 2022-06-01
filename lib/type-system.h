#ifndef NUJEL_LIB_TYPE_SYSTEM
#define NUJEL_LIB_TYPE_SYSTEM

#include "nujel.h"
#include "type/vec.h"

i64             castToInt   (const lVal *v, i64 fallback);
bool            castToBool  (const lVal *v);
const char *    castToString(const lVal *v, const char *fallback);

NORETURN void   throwTypeError      (lClosure *c, lVal *v, lType T);
NORETURN void   throwArityError     (lClosure *c, lVal *v, int arity);
vec             requireVec          (lClosure *c, lVal *v);
vec             requireVecCompatible(lClosure *c, lVal *v);
i64             requireInt          (lClosure *c, lVal *v);
i64             requireNaturalInt   (lClosure *c, lVal *v);
double          requireFloat        (lClosure *c, lVal *v);
lArray *        requireArray        (lClosure *c, lVal *v);
const lSymbol * requireSymbol       (lClosure *c, lVal *v);
const lSymbol * requireKeyword      (lClosure *c, lVal *v);
const lSymbol * requireSymbolic     (lClosure *c, lVal *v);
lString *       requireString       (lClosure *c, lVal *v);
lTree *         requireTree         (lClosure *c, lVal *v);
lTree *         requireMutableTree  (lClosure *c, lVal *v);
lBytecodeOp     requireBytecodeOp   (lClosure *c, lVal *v);
lBytecodeArray *requireBytecodeArray(lClosure *c, lVal *v);
lClosure       *requireClosure      (lClosure *c, lVal *v);
lVal           *requireEnvironment  (lClosure *c, lVal *v);

lType lTypecast         (const lType a, const lType b);

static inline bool isComment(lVal *v){return v && v->type == ltComment;}


#endif
