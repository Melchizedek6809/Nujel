#pragma once
#include "../nujel.h"

#define SYM_MAX (1<<14)
extern lSymbol  lSymbolList [SYM_MAX];
extern uint     lSymbolMax;

extern lSymbol *symNull,*symQuote,*symQuasiquote, *symUnquote, *symUnquoteSplicing, *symArr,*symIf,*symCond,*symDo,*symMinus,*symLambda,*symLambdAst,*symTreeNew;

void      lSymbolInit  ();
lSymbol  *lSymS        (const char *s);
lSymbol  *lSymSL       (const char *s, uint len);
lSymbol  *getTypeSymbol(const lVal *a);