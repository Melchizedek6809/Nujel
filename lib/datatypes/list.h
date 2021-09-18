#pragma once
#include "../nujel.h"

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
