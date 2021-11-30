#pragma once
#include "../nujel.h"
#include "../collection/list.h"
#include "../type-system.h"

lVal     *lValBool  (bool v);
lVal     *lValInt   (int v);
lVal     *lValFloat (float v);
lVal     *lValVec   (const vec v);
lVal     *lValTree  (lTree *v);
lVal     *lValObject(lClosure *v);
lVal     *lValLambda(lClosure *v);
lVal     *lValCopy  (lVal *dst, const lVal *src);
lVal     *lValDup   (const lVal *v);
