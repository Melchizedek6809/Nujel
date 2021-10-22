#pragma once
#include "../nujel.h"

#define TRE_MAX (1<<17)

extern lTree  lTreeList[TRE_MAX];
extern uint   lTreeMax;
extern uint   lTreeActive;
extern lTree *lTreeFFree;

void   lTreeInit         ();
lTree *lTreeAlloc        ();
lTree *lTreeNew          (const lSymbol *s, lVal *v);
void   lTreeFree         (lTree *t);

lVal  *lTreeAddToList       (const lTree *t, lVal *v);
lVal  *lTreeAddKeysToList   (const lTree *t, lVal *v);
lVal  *lTreeAddValuesToList (const lTree *t, lVal *v);
lVal  *lTreeToList          (const lTree *t);
lVal  *lTreeKeysToList      (const lTree *t);
lVal  *lTreeValuesToList    (const lTree *t);
lVal  *lTreeGet             (const lTree *t, const lSymbol *s);
bool   lTreeHas             (const lTree *t, const lSymbol *s);
void   lTreeInsert          (      lTree *t, const lSymbol *s, lVal *v);
void   lTreeSet             (      lTree *t, const lSymbol *s, lVal *v);
