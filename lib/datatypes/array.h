#pragma once
#include "../nujel.h"

typedef struct {
	u32 *data;
	i32 length;
	u16 flags;
	u16 nextFree;
} lArray;

#define ARR_MAX (1<<12)
#define ARR_MASK ((ARR_MAX)-1)
extern uint   lArrayActive;
extern uint   lArrayMax;
extern lArray lArrayList[ARR_MAX];

#define lArrNull(val)   (((val->vCdr & ARR_MASK) == 0) || (lArrayList[val->vCdr & ARR_MASK].data == NULL))
#define lArr(val)       lArrayList[val->vCdr & ARR_MASK]
#define lArrLength(val) lArr(val).length
#define lArrData(val)   lArr(val).data

void lInitArray    ();
uint lArrayAlloc   ();
void lArrayFree    (uint v);
