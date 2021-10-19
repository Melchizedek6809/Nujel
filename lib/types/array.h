#pragma once
#include "../nujel.h"

struct lArray {
	union {
		lVal **data;
		lArray *nextFree;
	};
	i32 length;
};
#define ARR_MAX (1<<12)

extern uint   lArrayActive;
extern uint   lArrayMax;
extern lArray lArrayList[ARR_MAX];

void    lInitArray    ();
lArray *lArrayAlloc   ();
void    lArrayFree    (lArray *v);
