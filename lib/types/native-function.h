#pragma once
#include "../nujel.h"

struct lNFunc {
	union {
		lVal *(*fp)(lClosure *, lVal *);
		lNFunc *nextFree;
	};
	lVal *doc;
	u16 flags;
};

#define NFN_MAX (1<<10)
#define NFN_MASK ((NFN_MAX)-1)

extern lNFunc   lNFuncList  [NFN_MAX];
extern uint     lNFuncMax;
extern uint     lNFuncActive;

int       lNFuncID(const lNFunc *n);
void      lInitNativeFunctions();
void      lNFuncFree(uint i);
lNFunc   *lNFuncAlloc();

lVal     *lAddNativeFunc    (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lAddSpecialForm   (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
