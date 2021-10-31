#pragma once
#include "../nujel.h"

#define ARR_MAX (1<<12)
extern lArray  lArrayList[ARR_MAX];
extern uint    lArrayActive;
extern uint    lArrayMax;
extern lArray *lArrayFFree;

void    lArrayInit    ();
lArray *lArrayAlloc   ();
void    lArrayFree    (lArray *v);
