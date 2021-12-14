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
#include "../type/closure.h"
#include "../type/val.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
	lType t;
	union {
		lClosure *vClosure;
		lVal     *vVal;
		lString  *vString;
		void     *vPointer;
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
			fprintf(stderr,"Can't grow rootsStack\n");
			exit(123);
		}
	}
	rootStack[rootSP++] = (rootEntry){t,{ptr}};
}

/* Push an lClosure onto the root stack, protecting it from being freed by the GC */
lClosure *lRootsClosurePush(lClosure *c){
	lRootsPush(ltLambda,c);
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

void (*rootsMarkerChain)() = NULL;
/* Mark every single root and everything they point to */
void lRootsMark(){
	for(int i=0;i<rootSP;i++){
		switch(rootStack[i].t){
		case ltLambda:
			lClosureGCMark(rootStack[i].vClosure);
			break;
		case ltPair:
			lValGCMark(rootStack[i].vVal);
			break;
		case ltString:
			lStringGCMark(rootStack[i].vString);
			break;
		default:
			break;
		}
	}
	if(rootsMarkerChain){rootsMarkerChain();}
}
