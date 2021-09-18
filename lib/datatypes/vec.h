#pragma once
#include "../nujel.h"

typedef struct {
	vec v;
	u16 nextFree;
	u16 flags;
} lVec;

#define VEC_MAX (1<<14)
#define VEC_MASK ((VEC_MAX)-1)

#define lVec(i)      lVecList[i & VEC_MASK]
#define lVecV(i)     lVec(i).v
#define lVecFlags(i) lVec(i).flags

extern lVec lVecList[VEC_MAX];
extern uint lVecActive;
extern uint lVecMax;

void  lInitVec  ();
uint  lVecAlloc ();
void  lVecFree  (uint i);
lVal *lValVec   (const vec v);
