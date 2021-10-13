#pragma once
#include "../nujel.h"

struct lSymbol {
	char c[32];
};

#define SYM_MAX (1<<14)
#define SYM_MASK ((SYM_MAX)-1)
extern lSymbol  lSymbolList [SYM_MAX];
extern uint     lSymbolMax;

#define lvSym(i)  (i == 0 ? symNull : &lSymbolList[i & SYM_MASK])
#define lvSymI(s) (s == NULL ? 0 : s - lSymbolList)
#define lGetSymbol(v) (((v == NULL) || (v->type != ltSymbol)) ? symNull : lvSym(v->vCdr))

extern lSymbol *symNull,*symQuote,*symArr,*symIf,*symCond,*symWhen,*symUnless,*symLet,*symDo,*symMinus,*symLambda,*symLambdAst;
extern lSymbol *lSymLTNoAlloc, *lSymLTBool, *lSymLTPair, *lSymLTLambda, *lSymLTInt, *lSymLTFloat, *lSymLTVec, *lSymLTString, *lSymLTSymbol, *lSymLTNativeFunction, *lSymLTSpecialForm, *lSymLTInfinity, *lSymLTArray, *lSymLTGUIWidget;

void      lInitSymbol  ();
lVal     *lValSymS     (const lSymbol *s);
lVal     *lValSym      (const char *s);
lSymbol  *lSymS        (const char *s);
lSymbol  *lSymSL       (const char *s, uint len);

int       lSymCmp      (const lVal *a,const lVal *b);
int       lSymEq       (const lSymbol *a,const lSymbol *b);

bool      lSymVariadic (const lSymbol *s);
bool      lSymNoEval   (const lSymbol *s);
