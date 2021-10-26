#pragma once
#include "../nujel.h"
#include "../collection/list.h"
#include "../type-system.h"

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

void      lInitVal();

lVal     *lValAlloc ();
void      lValFree  (lVal *v);

lVal     *lValBool  (bool v);
lVal     *lValInf   ();
lVal     *lValInt   (int v);
lVal     *lValFloat (float v);
lVal     *lValTree  (lTree *v);
lVal     *lValCopy  (lVal *dst, const lVal *src);
lVal     *lValDup   (const lVal *v);
