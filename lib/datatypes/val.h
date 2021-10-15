#pragma once
#include "../nujel.h"

struct lVal {
	u8 flags;
	u8 type;
	union {
		bool       vBool;
		lPair      vList;
		int        vInt;
		float      vFloat;
		lVec      *vVec;
		lArray    *vArray;
		lString   *vString;
		lSymbol   *vSymbol;
		lClosure  *vClosure;
		lNFunc    *vNFunc;
		void      *vPointer;

		lVal      *nextFree;
	};
};
#define lfMarked    ( 1)
#define lfNoGC      ( 2)
#define lfConst     ( 4)
#define lfInUse     ( 8)

#define VAL_MAX (1<<20)
#define VAL_MASK ((VAL_MAX)-1)

extern lVal     lValList    [VAL_MAX];
extern uint     lValMax;
extern uint     lValActive;

#define forEach(n,v) for(lVal *n = v;(n != NULL) && (n->type == ltPair) && (n->vList.car != NULL); n = n->vList.cdr)

void lInitVal();

lVal     *lValAlloc ();
void      lValFree  (lVal *v);

lVal     *lValBool  (bool v);
lVal     *lValInf   ();
lVal     *lValInt   (int v);
lVal     *lValFloat (float v);
lVal     *lValCopy  (lVal *dst, const lVal *src);
lVal     *lValDup   (const lVal *v);
