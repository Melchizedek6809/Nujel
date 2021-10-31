#pragma once
#include "../nujel.h"

#define TRE_MAX (1<<18)
extern lTree  lTreeList[TRE_MAX];
extern uint   lTreeMax;
extern uint   lTreeActive;
extern lTree *lTreeFFree;

void   lTreeInit ();
lTree *lTreeAlloc();
void   lTreeFree (lTree *t);
