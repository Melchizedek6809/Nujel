#pragma once
#include "../nujel.h"

#define VAL_MAX (1<<21)
extern lVal  lValList[VAL_MAX];
extern uint  lValMax;
extern uint  lValActive;
extern lVal *lValFFree;

void      lValInit  ();
lVal     *lValAlloc ();
void      lValFree  (lVal *v);
