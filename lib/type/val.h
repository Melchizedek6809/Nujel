#ifndef NUJEL_LIB_TYPE_VAL
#define NUJEL_LIB_TYPE_VAL
#include "../nujel.h"
#include "../type-system.h"

int       lValCompare(const lVal *a, const lVal *b);
bool      lValEqual  (const lVal *a, const lVal *b);
i64       lValGreater(const lVal *a, const lVal *b);
lVal     *lValBool   (bool v);
lVal     *lValInt    (i64 v);
lVal     *lValFloat  (double v);
lVal     *lValVec    (const vec v);
lVal     *lValTree   (lTree *v);
lVal     *lValObject (lClosure *v);
lVal     *lValLambda (lClosure *v);

static inline lVal *lValComment(){return lValAlloc(ltComment);}

#endif
