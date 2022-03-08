/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 *
 * Contains code for dealing with GC Roots, essential for determining which
 * objects on the heap are still reachable.
 */
#include "roots.h"
#include "../type-system.h"
#include "../allocation/garbage-collection.h"
#include "../collection/string.h"
#include "../misc/pf.h"
#include "../type/closure.h"
#include "../type/val.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	lType t;
	union {
		lClosure *vClosure;
		lVal     *vVal;
		lTree    *vTree;
		lString  *vString;
		lSymbol  *vSymbol;
		void     *vPointer;
		lVal     **vValRef;
		lClosure **vClosureRef;
	};
} rootEntry;

rootEntry *rootStack = NULL;
int rootSP  = 0;
int rootMax = 0;

/* Push a new pointer onto the root stack */
static void lRootsPush(const lType t, void *ptr){
	if(rootSP >= rootMax){
		rootMax = MAX(rootMax * 2, 256);
		rootStack = realloc(rootStack, rootMax * sizeof(rootEntry));
		if(rootStack == NULL){
			fpf(stderr,"Can't grow rootsStack\n");
			exit(123);
		}
	}
	rootStack[rootSP].t = t;
	rootStack[rootSP].vPointer = ptr;
	rootSP++;
}

/* Push an lClosure onto the root stack, protecting it from being freed by the GC */
lClosure *lRootsClosurePush(lClosure *c){
	lRootsPush(ltLambda,c);
	return c;
}

/* Push an lClosure onto the root stack, protecting it from being freed by the GC */
lTree *lRootsTreePush(lTree *c){
	lRootsPush(ltTree,c);
	return c;
}

/* Push an lVal onto the root stack, protecting it from being freed by the GC */
lVal *lRootsValPush(lVal *v){
	lRootsPush(ltPair,v);
	return v;
}

/* Push an lString onto the root stack, protecting it from being freed by the GC */
lString *lRootsStringPush(lString *s){
	lRootsPush(ltString,s);
	return s;
}

/* Push an lString onto the root stack, protecting it from being freed by the GC */
lSymbol *lRootsSymbolPush(lSymbol *s){
	lRootsPush(ltSymbol,s);
	return s;
}

void lRootsBytecodePush(lVal *start){
	lRootsPush(ltBytecodeOp, start);
}

void lRootsValStackPush  (lVal **c){
	lRootsPush(ltValueStack, c);
}

void lRootsCallStackPush (lClosure **c){
	lRootsPush(ltCallStack, c);
}

void (*rootsMarkerChain)() = NULL;
/* Mark every single root and everything they point to */
void lRootsMark(){
	for(int i=0;i<rootSP;i++){
		switch(rootStack[i].t){
		case ltValueStack:
			lValStackGCMark(rootStack[i].vValRef);
			break;
		case ltCallStack:
			lCallStackGCMark(rootStack[i].vClosureRef);
			break;
		case ltLambda:
			lClosureGCMark(rootStack[i].vClosure);
			break;
		case ltPair:
			lValGCMark(rootStack[i].vVal);
			break;
		case ltSymbol:
			lSymbolGCMark(rootStack[i].vSymbol);
			break;
		case ltString:
			lStringGCMark(rootStack[i].vString);
			break;
		case ltTree:
			lTreeGCMark(rootStack[i].vTree);
			break;
		case ltBytecodeOp:
			lBytecodeStackMark(rootStack[i].vPointer);
		default:
			break;
		}
	}
	if(rootsMarkerChain){rootsMarkerChain();}
}
