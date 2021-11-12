#pragma once
#include "../nujel.h"
#include "../type/val.h"

lVal     *lCons       (lVal *car,lVal *cdr);
int       lListLength (lVal *v);

static inline lVal *lCar(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.car : NULL;
}
static inline lVal *lCdr(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.cdr : NULL;
}
static inline lVal *lCaar (lVal *v){return lCar(lCar(v));}
static inline lVal *lCadr (lVal *v){return lCar(lCdr(v));}
static inline lVal *lCdar (lVal *v){return lCdr(lCar(v));}
static inline lVal *lCddr (lVal *v){return lCdr(lCdr(v));}
static inline lVal *lCadar(lVal *v){return lCar(lCdr(lCar(v)));}
static inline lVal *lCaddr(lVal *v){return lCar(lCdr(lCdr(v)));}
static inline lVal *lCdddr(lVal *v){return lCdr(lCdr(lCdr(v)));}

#define forEach(n,v) for(lVal *n = v;(n != NULL) && (n->type == ltPair) && (n->vList.car != NULL); n = n->vList.cdr)
