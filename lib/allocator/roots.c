/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 *
 * Contains code for dealing with GC Roots, essential for determining which
 * objects on the heap are still reachable.
 */
#include "roots.h"
#include "garbage-collection.h"

#ifndef COSMOPOLITAN_H_
	#include <stdio.h>
	#include <stdlib.h>
#endif

typedef struct rootLink rootLink;
struct rootLink {
	void *entry;
	rootLink *next;
};
rootLink *freeLink     = NULL;

rootLink *rootsClosure = NULL;
rootLink *rootsVal     = NULL;

void rootLinkAllocFromHeap(){
	rootLink *block = malloc(sizeof(rootLink) * 256);
	if(block == NULL){
		fprintf(stderr,"rootLink OOM\n");
		exit(13);
	}
	for(int i=255;i>=0;i--){
		block[i].entry = freeLink;
		freeLink = &block[i];
	}
}

rootLink *rootLinkAlloc(){
	if(freeLink == NULL){
		rootLinkAllocFromHeap();
	}
	rootLink *ret = freeLink;
	freeLink = ret->entry;
	return ret;
}

void rootLinkFree(rootLink *l){
	if(l == NULL){return;}
	l->entry = freeLink;
	freeLink = l;
}


void lRootsClosurePush(const lClosure *c){
	rootLink *ret = rootLinkAlloc();
	ret->next     = rootsClosure;
	ret->entry    = (void *)c;
	rootsClosure  = ret;
}

void lRootsClosurePop(){
	if(rootsClosure == NULL){return;}
	rootsClosure = rootsClosure->next;
}

void lRootsClosureMark(){
	for(rootLink *l = rootsClosure; l != NULL; l = l->next){
		lClosureGCMark((const lClosure *)l->entry);
	}
}

void lRootsValPush(lVal *c){
	rootLink *ret = rootLinkAlloc();
	ret->next     = rootsVal;
	ret->entry    = (void *)c;
	rootsVal  = ret;
}

void lRootsValPop(){
	if(rootsVal == NULL){return;}
	rootsVal = rootsVal->next;
}

void lRootsValMark(){
	for(rootLink *l = rootsVal; l != NULL; l = l->next){
		lValGCMark((lVal *)l->entry);
	}
}
