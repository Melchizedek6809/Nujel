/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 *
 * Contains code for dealing with GC Roots, essential for determining which
 * objects on the heap are still reachable.
 */
#include "roots.h"
#include "../collection/closure.h"
#include "garbage-collection.h"

#include <stdio.h>
#include <stdlib.h>

lClosure **rootsClosure = NULL;
uint rootsClosureSP     = 0;
uint rootsClosureMax    = 0;

lVal **rootsVal  = NULL;
uint rootsValSP  = 0;
uint rootsValMax = 0;

lString **rootsString = NULL;
uint rootsStringSP    = 0;
uint rootsStringMax   = 0;

lClosure *lRootsClosurePush(lClosure *c){
	if(rootsClosureSP >= rootsClosureMax){
		rootsClosureMax = MAX(rootsClosureMax * 2, 256);
		rootsClosure = realloc(rootsClosure, rootsClosureMax * sizeof(lClosure *));
		if(rootsClosure == NULL){
			fprintf(stderr,"Can't grow rootsClosure\n");
			exit(123);
		}
	}
	rootsClosure[rootsClosureSP++] = c;
	return c;
}

lClosure *lRootsClosurePop(){
	if(rootsClosureSP == 0){
		fprintf(stderr,"rootsClosure underflow\n");
		exit(124);
	}
	return rootsClosure[--rootsClosureSP];
}

void lRootsClosureMark(){
	for(uint i=0;i<rootsClosureSP;i++){
		lClosureGCMark(rootsClosure[i]);
	}
}

lVal *lRootsValPush(lVal *c){
	if(rootsValSP >= rootsValMax){
		rootsValMax = MAX(rootsValMax * 2, 256);
		rootsVal = realloc(rootsVal, rootsValMax * sizeof(lVal *));
		if(rootsVal == NULL){
			fprintf(stderr,"Can't grow rootsVal\n");
			exit(123);
		}
	}
	rootsVal[rootsValSP++] = c;
	return c;
}

lVal *lRootsValPop(){
	if(rootsValSP == 0){
		fprintf(stderr,"rootsVal underflow\n");
		exit(124);
	}
	return rootsVal[--rootsValSP];
}

void lRootsValMark(){
	for(uint i=0;i<rootsValSP;i++){
		lValGCMark(rootsVal[i]);
	}
}

lString *lRootsStringPush(lString *s){
	if(rootsStringSP >= rootsStringMax){
		rootsStringMax = MAX(rootsStringMax * 2, 256);
		rootsString = realloc(rootsString, rootsStringMax * sizeof(lString *));
		if(rootsString == NULL){
			fprintf(stderr,"Can't grow rootsString\n");
			exit(123);
		}
	}
	rootsString[rootsStringSP++] = s;
	return s;
}

lString *lRootsStringPop(){
	if(rootsStringSP == 0){
		fprintf(stderr,"rootsString underflow\n");
		exit(124);
	}
	return rootsString[--rootsStringSP];
}

void lRootsStringMark(){
	for(uint i=0;i<rootsStringSP;i++){
		lStringGCMark(rootsString[i]);
	}
}
