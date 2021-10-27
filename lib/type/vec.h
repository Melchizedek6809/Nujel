#pragma once
#include "../nujel.h"
#include "val.h"

#define VEC_MAX (1<<14)
extern lVec lVecList[VEC_MAX];
extern uint lVecActive;
extern uint lVecMax;

void  lInitVec  ();
lVec *lVecAlloc ();
void  lVecFree  (lVec *v);
lVal *lValVec   (const vec v);
