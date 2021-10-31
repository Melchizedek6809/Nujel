#pragma once
#include "../nujel.h"

#define TRE_MAX (1<<18)
extern lTree  lTreeList[TRE_MAX];
extern uint   lTreeMax;
extern uint   lTreeActive;
extern lTree *lTreeFFree;

void   lTreeInit            ();
void   lTreeFree            (      lTree *t);

lVal  *lTreeGet             (const lTree *t, const lSymbol *s, bool *found);
bool   lTreeHas             (const lTree *t, const lSymbol *s, lVal **value);
void   lTreeSet             (      lTree *t, const lSymbol *s, lVal *v, bool *found);
lTree *lTreeInsert          (      lTree *t, const lSymbol *s, lVal *v);

lVal  *lTreeAddToList       (const lTree *t, lVal *v);
lVal  *lTreeAddKeysToList   (const lTree *t, lVal *v);
lVal  *lTreeAddValuesToList (const lTree *t, lVal *v);
lVal  *lTreeToList          (const lTree *t);
lVal  *lTreeKeysToList      (const lTree *t);
lVal  *lTreeValuesToList    (const lTree *t);

uint   lTreeSize            (const lTree *t);