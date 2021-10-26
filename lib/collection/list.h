#pragma once
#include "../nujel.h"

typedef struct {
	lVal *car,*cdr;
} lPair;

lVal     *lCons       (lVal *car,lVal *cdr);
lVal     *lCar        (lVal *v);
lVal     *lCdr        (lVal *v);
lVal     *lCaar       (lVal *v);
lVal     *lCadr       (lVal *v);
lVal     *lCdar       (lVal *v);
lVal     *lCddr       (lVal *v);
lVal     *lCadar      (lVal *v);
lVal     *lCaddr      (lVal *v);
lVal     *lCdddr      (lVal *v);
lVal     *lLastCar    (lVal *v);
int       lListLength (lVal *v);

#define forEach(n,v) for(lVal *n = v;(n != NULL) && (n->type == ltPair) && (n->vList.car != NULL); n = n->vList.cdr)
