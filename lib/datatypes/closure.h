#pragma once
#include "../nujel.h"

struct lClosure {
	lVal *data;
	lVal *text;
	u16 parent;
	u16 nextFree;
	u16 flags;
	u16 refCount;
};
#define lfDynamic   (16)
#define lfObject    (32)
#define lfUsed      (64)

#define CLO_MAX (1<<16)
#define CLO_MASK ((CLO_MAX)-1)

extern lClosure lClosureList[CLO_MAX];
extern uint     lClosureMax;
extern uint     lClosureActive;

#define lClo(i)       lClosureList[i & CLO_MASK]
#define lCloI(c)      (c == NULL ? 0 : c - lClosureList)
#define lCloParent(i) lClosureList[i & CLO_MASK].parent
#define lCloData(i)   lClosureList[i & CLO_MASK].data
#define lCloText(i)   lClosureList[i & CLO_MASK].text

void      lInitClosure      ();
uint      lClosureAlloc     ();
uint      lClosureNew       (uint parent);
void      lClosureFree      (uint c);

lVal     *lSearchClosureSym (uint c, lVal *ret, const char *str, uint len);
lVal     *lResolve          (lClosure *c, lVal *v);
lVal     *lResolveSym       (uint c, lVal *v);
void      lDefineVal        (lClosure *c, const char *str, lVal *val);
lVal     *lDefineAliased    (lClosure *c, lVal *lNF, const char *sym);

lVal     *lGetClosureSym    (uint      c, lSymbol *s);
lVal     *lResolveClosureSym(uint      c, lSymbol *s);
lVal     *lDefineClosureSym (uint      c, lSymbol *s);
// lVal     *lMatchClosureSym  (uint      c, lVal *v, lSymbol *s);
lVal     *lSearchClosureSym (uint      c, lVal *v, const char *str, uint len);
