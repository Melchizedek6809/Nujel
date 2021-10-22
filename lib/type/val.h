#pragma once
#include "../nujel.h"

struct lVal {
	u8 type;
	union {
		bool           vBool;
		lPair          vList;
		int            vInt;
		float          vFloat;
		lVec          *vVec;
		lArray        *vArray;
		lTree         *vTree;
		lString       *vString;
		const lSymbol *vSymbol;
		lClosure      *vClosure;
		lNFunc        *vNFunc;
		void          *vPointer;

		lVal          *nextFree;
	};
};

#define VAL_MAX (1<<20)

extern lVal  lValList[VAL_MAX];
extern uint  lValMax;
extern uint  lValActive;
extern lVal *lValFFree;

#define forEach(n,v) for(lVal *n = v;(n != NULL) && (n->type == ltPair) && (n->vList.car != NULL); n = n->vList.cdr)

void      lInitVal();

lVal     *lValAlloc ();
void      lValFree  (lVal *v);

lVal     *lValBool  (bool v);
lVal     *lValInf   ();
lVal     *lValInt   (int v);
lVal     *lValFloat (float v);
lVal     *lValCopy  (lVal *dst, const lVal *src);
lVal     *lValDup   (const lVal *v);
