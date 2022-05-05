#ifndef NUJEL_LIB_TYPE_SYSTEM
#define NUJEL_LIB_TYPE_SYSTEM

#include "nujel.h"
#include "misc/vec.h"

vec         requireVec  (lClosure *c, lVal *v);
i64         requireInt  (lClosure *c, lVal *v);
double      requireFloat(lClosure *c, lVal *v);

lVal *lCast             (lClosure *c, lVal *v, lType t);
lVal *lCastAuto         (lClosure *c, lVal *v);
lType lTypecast         (const lType a, const lType b);

void  lOperationsTypeSystem(lClosure *c);

static inline bool isComment(lVal *v){return v && v->type == ltComment;}

/* Cast v to be an int without memory allocations, or return fallback */
static inline i64 castToInt(const lVal *v, i64 fallback){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v->vVec.x;
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	default:
		return fallback;
	}
}


/* Cast v to be a float without memory allocations, or return fallback */
static inline double castToFloat(const lVal *v, double fallback){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v->vVec.x;
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	default:
		return fallback;
	}
}

/* Cast v to be a vec without memory allocations, or return fallback */
static inline vec castToVec(const lVal *v, vec fallback){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v->vVec;
	case ltFloat:
		return vecNew(v->vFloat,v->vFloat,v->vFloat);
	case ltInt:
		return vecNew(v->vInt,v->vInt,v->vInt);
	default:
		return fallback;
	}
}

/* Cast v to be a bool without memory allocations, or return false */
static inline bool castToBool(const lVal *v){
	if(v == NULL){
		return false;
	}else if(v->type == ltBool){
		return v->vBool;
	}else{
		return true;
	}
}

/* Cast v to be a string without memory allocations, or return fallback */
static inline const char *castToString(const lVal *v, const char *fallback){
	if((v == NULL) || (v->type != ltString)){return fallback;}
	return v->vString->data;
}

/* Return the tree in V if possible, otherwise fallback. */
static inline lTree *castToTree(const lVal *v, lTree *fallback){
	if((v == NULL) || (v->type != ltTree)){return fallback;}
	return v->vTree;
}

/* Return the tree in V if possible, otherwise fallback. */
static inline const lSymbol *castToSymbol(const lVal *v, const lSymbol *fallback){
	if((v == NULL) || ((v->type != ltSymbol) && (v->type != ltKeyword))){return fallback;}
	return v->vSymbol;
}

#endif
