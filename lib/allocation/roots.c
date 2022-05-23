/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */

/*
 * Contains code for dealing with GC Roots, essential for determining which
 * objects on the heap are still reachable.
 */
#include "roots.h"
#include "../printer.h"

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
		lThread  *vThread;
	};
} rootEntry;

rootEntry *rootStack = NULL;
int rootSP  = 0;
int rootMax = 0;

void (*rootsMarkerChain)() = NULL;

static void *lRootsPush(const lType t, void *ptr){
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
	return ptr;
}

/* Push an lClosure onto the root stack, protecting it from being freed by the GC */
lClosure *lRootsClosurePush(lClosure *c){
	return lRootsPush(ltLambda,c);
}

/* Push an lClosure onto the root stack, protecting it from being freed by the GC */
lTree *lRootsTreePush(lTree *c){
	return lRootsPush(ltTree,c);
}

/* Push an lVal onto the root stack, protecting it from being freed by the GC */
lVal *lRootsValPush(lVal *v){
	return lRootsPush(ltPair,v);
}

/* Push an lString onto the root stack, protecting it from being freed by the GC */
lString *lRootsStringPush(lString *s){
	return lRootsPush(ltString,s);
}

/* Push an lString onto the root stack, protecting it from being freed by the GC */
lSymbol *lRootsSymbolPush(lSymbol *s){
	return lRootsPush(ltSymbol,s);
}

/* Push an lThread onto the root stack, protecting it from being freed by the GC */
void lRootsThreadPush(lThread *c){
	lRootsPush(ltThread, c);
}

/* Mark every single root and everything they point to */
void lRootsMark(){
	for(int i=0;i<rootSP;i++){
		switch(rootStack[i].t){
		case ltThread:
			lThreadGCMark(rootStack[i].vThread);
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
		default:
			break;
		}
	}
	if(rootsMarkerChain){rootsMarkerChain();}
}
