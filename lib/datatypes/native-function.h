#pragma once
#include "../nujel.h"

typedef struct {
	lVal *(*fp)(lClosure *, lVal *);
	lVal *doc;
	u16 flags;
	u16 nextFree;
} lNFunc;

#define NFN_MAX (1<<10)
#define NFN_MASK ((NFN_MAX)-1)

extern lNFunc   lNFuncList  [NFN_MAX];
extern uint     lNFuncMax;
extern uint     lNFuncActive;

void      lInitNativeFunctions();
void      lNFuncFree(uint i);
uint      lNFuncAlloc();

lVal     *lAddNativeFunc    (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lAddSpecialForm   (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lValNativeFunc    (lVal *(*func)(lClosure *,lVal *), lVal *args, lVal *docString);

#define lNFN(i) lNFuncList[i & NFN_MASK]
