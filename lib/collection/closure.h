#pragma once
#include "../nujel.h"

typedef enum closureType {
	closureDefault = 0,
	closureObject = 1,
	closureConstant = 2
} closureType;

struct lClosure {
	union {
		lClosure *parent;
		lClosure *nextFree;
	};
	lVal *data;
	lVal *text;
	lVal *doc;
	u8 type;
};

#define CLO_MAX (1<<16)

extern lClosure lClosureList[CLO_MAX];
extern uint     lClosureMax;
extern uint     lClosureActive;
extern lClosure *lClosureFFree;

void      lInitClosure      ();
lClosure *lClosureAlloc     ();
lClosure *lClosureNew       (lClosure *parent);
void      lClosureFree      (lClosure *c);
int       lClosureID        (const lClosure *n);

lVal     *lSearchClosureSym (lClosure *c, lVal *ret, const char *str, uint len);
lVal     *lResolve          (lClosure *c, lVal *v);
lVal     *lResolveSym       (lClosure *c, lVal *v);
void      lDefineVal        (lClosure *c, const char *str, lVal *val);
lVal     *lDefineAliased    (lClosure *c, lVal *lNF, const char *sym);

lVal     *lGetClosureSym    (lClosure *c, const lSymbol *s);
lVal     *lDefineClosureSym (lClosure *c, const lSymbol *s);
lVal     *lSearchClosureSym (lClosure *c, lVal *v, const char *str, uint len);
