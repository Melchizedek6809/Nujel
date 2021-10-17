#pragma once
#include "../nujel.h"
#include "val.h"

struct lVec {
	union {
		vec v;
		lVec *nextFree;
	};
	u16 flags;
};

#define VEC_MAX (1<<14)
#define VEC_MASK ((VEC_MAX)-1)

extern lVec lVecList[VEC_MAX];
extern uint lVecActive;
extern uint lVecMax;

void  lInitVec  ();
lVec *lVecAlloc ();
void  lVecFree  (uint i);
lVal *lValVec   (const vec v);
