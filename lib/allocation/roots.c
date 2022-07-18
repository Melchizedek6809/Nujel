/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */

/*
 * Contains code for dealing with GC Roots, essential for determining which
 * objects on the heap are still reachable.
 */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

#include <stdlib.h>

typedef struct {
	lType t;
	union {
		lClosure *vClosure;
		lVal     *vVal;
		lTree    *vTree;
		lSymbol  *vSymbol;
		void     *vPointer;
		lThread  *vThread;
	};
} rootEntry;

rootEntry *rootStack = NULL;
int rootSP  = 0;
int rootMax = 0;

static void *lRootsPush(const lType t, void *ptr){
	if(unlikely(rootSP >= rootMax)){
		rootMax = MAX(rootMax * 2, 256);
		rootEntry *newRootStack = realloc(rootStack, rootMax * sizeof(rootEntry));
		if(unlikely(newRootStack == NULL)){
			free(rootStack);
			epf("Can't grow rootStack\n");
			exit(123);
		}
		rootStack = newRootStack;
	}
	rootStack[rootSP].t = t;
	rootStack[rootSP].vPointer = ptr;
	rootSP++;
	return ptr;
}

lVal *lRootsValPush(lVal *v){
	return lRootsPush(ltPair, v);
}
lClosure *lRootsClosurePush(lClosure *v){
	return lRootsPush(ltLambda, v);
}
lSymbol *lRootsSymbolPush(lSymbol *v){
	return lRootsPush(ltSymbol, v);
}
lThread *lRootsThreadPush(lThread *v){
	return lRootsPush(ltThread, v);
}

/* Mark every single root and everything they point to */
void lRootsMark(){
	//pf("rootSP: %i\n", (i64)rootSP);
	for(int i=0;i<rootSP;i++){
		switch(rootStack[i].t){
		case ltSymbol:
			lSymbolGCMark(rootStack[i].vSymbol);
			break;
		case ltLambda:
			lClosureGCMark(rootStack[i].vClosure);
			break;
		case ltPair:
			lValGCMark(rootStack[i].vVal);
			break;
		case ltThread:
			lThreadGCMark(rootStack[i].vThread);
			break;
		default:
			break;
		}
	}
}
