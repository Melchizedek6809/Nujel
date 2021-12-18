#pragma once
#include "../nujel.h"
#include "../collection/list.h"
#include "../type-system.h"

lVal     *lValBool   (bool v);
lVal     *lValInt    (i64 v);
lVal     *lValFloat  (double v);
lVal     *lValVec    (const vec v);
lVal     *lValTree   (lTree *v);
lVal     *lValObject (lClosure *v);
lVal     *lValLambda (lClosure *v);
lVal     *lValComment();
