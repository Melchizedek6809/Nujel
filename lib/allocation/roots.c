/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 *
 * Contains code for dealing with GC Roots, essential for determining which
 * objects on the heap are still reachable.
 */
#include "roots.h"
#include "../type-system.h"
#include "../type/val.h"
#include "../collection/string.h"
#include "../collection/closure.h"
#include "garbage-collection.h"

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

lClosure *lRootsClosurePush(lClosure *c){
	lRootsPush(ltLambda,c);
	return c;
}

lVal *lRootsValPush(lVal *v){
	lRootsPush(ltPair,v);
	return v;
}

lString *lRootsStringPush(lString *s){
	lRootsPush(ltString,s);
	return s;
}

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
}
