#pragma once
#include "../nujel.h"

struct lSymbol {
	char c[32];
};

#define SYM_MAX (1<<14)
extern lSymbol  lSymbolList [SYM_MAX];
extern uint     lSymbolMax;

#define lGetSymbol(v) (((v == NULL) || (v->type != ltSymbol)) ? symNull : v->vSymbol)

extern lSymbol *symNull,*symQuote,*symArr,*symIf,*symCond,*symWhen,*symUnless,*symLet,*symDo,*symMinus,*symLambda,*symLambdAst;
extern lSymbol *lSymLTNoAlloc, *lSymLTBool, *lSymLTPair, *lSymLTLambda, *lSymLTInt, *lSymLTFloat, *lSymLTVec, *lSymLTString, *lSymLTSymbol, *lSymLTNativeFunction, *lSymLTSpecialForm, *lSymLTInfinity, *lSymLTArray, *lSymLTGUIWidget;

void      lInitSymbol  ();
lVal     *lValSymS     (lSymbol *s);
lVal     *lValSym      (const char *s);
lSymbol  *lSymS        (const char *s);
lSymbol  *lSymSL       (const char *s, uint len);
lSymbol  *getTypeSymbol(const lVal *a);

int       lSymCmp      (const lVal *a,const lVal *b);
int       lSymEq       (const lSymbol *a,const lSymbol *b);

bool      lSymVariadic (const lSymbol *s);
bool      lSymNoEval   (const lSymbol *s);
bool      lSymKeyword  (const lSymbol *s);
