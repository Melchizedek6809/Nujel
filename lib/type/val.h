#pragma once
#include "../nujel.h"
#include "../collection/list.h"
#include "../type-system.h"

#define VAL_MAX (1<<20)
extern lVal  lValList[VAL_MAX];
extern uint  lValMax;
extern uint  lValActive;
extern lVal *lValFFree;

void      lInitVal();

lVal     *lValAlloc ();
void      lValFree  (lVal *v);

lVal     *lValBool  (bool v);
lVal     *lValInf   ();
lVal     *lValInt   (int v);
lVal     *lValFloat (float v);
lVal     *lValVec   (const vec v);
lVal     *lValTree  (lTree *v);
lVal     *lValCopy  (lVal *dst, const lVal *src);
lVal     *lValDup   (const lVal *v);
