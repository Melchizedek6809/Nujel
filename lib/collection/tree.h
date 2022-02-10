#ifndef NUJEL_LIB_COLLECTION_TREE
#define NUJEL_LIB_COLLECTION_TREE
#include "../nujel.h"

lVal  *lTreeGet             (const lTree *t, const lSymbol *s, bool *found);
bool   lTreeHas             (const lTree *t, const lSymbol *s, lVal **value);
void   lTreeSet             (      lTree *t, const lSymbol *s, lVal *v, bool *found);
lTree *lTreeInsert          (      lTree *t, const lSymbol *s, lVal *v);
lTree *lTreeDup             (const lTree *t);

lVal  *lTreeAddToList       (const lTree *t, lVal *v);
lVal  *lTreeAddKeysToList   (const lTree *t, lVal *v);
lVal  *lTreeAddValuesToList (const lTree *t, lVal *v);
lVal  *lTreeToList          (const lTree *t);
lVal  *lTreeKeysToList      (const lTree *t);
lVal  *lTreeValuesToList    (const lTree *t);

uint   lTreeSize            (const lTree *t);

#endif
