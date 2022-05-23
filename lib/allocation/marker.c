/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "marker.h"
#include "garbage-collection.h"
#include "../common.h"
#include "../printer.h"

void *lStartOfStack;
void *lStartOfHeap;
void *lEndOfHeap;

void lSetStartOfStack(void *p){
	lStartOfStack = p;

	lStartOfHeap = lArrayList;
	lStartOfHeap = MIN(lStartOfHeap, (void *)lClosureList);
	lStartOfHeap = MIN(lStartOfHeap, (void *)lNFuncList);
	lStartOfHeap = MIN(lStartOfHeap, (void *)lStringList);
	lStartOfHeap = MIN(lStartOfHeap, (void *)lTreeList);
	lStartOfHeap = MIN(lStartOfHeap, (void *)lValList);
	lStartOfHeap = MIN(lStartOfHeap, (void *)lBytecodeArrayList);

	lEndOfHeap = &lArrayList[ARR_MAX];
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lClosureList[CLO_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lNFuncList[NFN_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lStringList[STR_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lTreeList[TRE_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lValList[VAL_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lBytecodeArrayList[BCA_MAX]);
}

static bool couldBeArray(void *p){
	if(p < (void *)lArrayList){return false;}
	if(p > (void *)&lArrayList[ARR_MAX]){return false;}
	if((intptr_t)p & (sizeof(void *) - 1)){return false;}
	return true;
}

static bool couldBeClosure(void *p){
	if(p < (void *)lClosureList){return false;}
	if(p > (void *)&lClosureList[CLO_MAX]){return false;}
	if((intptr_t)p & (sizeof(void *) - 1)){return false;}
	return true;
}

static bool couldBeNFunc(void *p){
	if(p < (void *)lNFuncList){return false;}
	if(p > (void *)&lNFuncList[NFN_MAX]){return false;}
	if((intptr_t)p & (sizeof(void *) - 1)){return false;}
	return true;
}

static bool couldBeString(void *p){
	if(p < (void *)lStringList){return false;}
	if(p > (void *)&lStringList[STR_MAX]){return false;}
	if((intptr_t)p & (sizeof(void *) - 1)){return false;}
	return true;
}

static bool couldBeTree(void *p){
	if(p < (void *)lTreeList){return false;}
	if(p > (void *)&lTreeList[TRE_MAX]){return false;}
	if((intptr_t)p & (sizeof(void *) - 1)){return false;}
	return true;
}

static bool couldBeVal(void *p){
	if(p < (void *)lValList){return false;}
	if(p > (void *)&lValList[VAL_MAX]){return false;}
	if((intptr_t)p & (sizeof(void *) - 1)){return false;}
	return true;
}

static bool couldBeBCA(void *p){
	if(p < (void *)lBytecodeArrayList){return false;}
	if(p > (void *)&lBytecodeArrayList[BCA_MAX]){return false;}
	if((intptr_t)p & (sizeof(void *) - 1)){return false;}
	return true;
}

void lGCMarkStack(void *endOfStack){
	const void *start = lStartOfStack;
	for(void *p = endOfStack; p < start; p+= sizeof(void *)){
		const u64 v = *((u64 *)p);
		if(v == 0xbaddbeefcafebabe){
			lThread *c = (lThread *)p;
			if((c->closureStackSize & 3) || (c->valueStackSize & 7)){continue;}
			if((c->closureStackSize > 0x10000) || (c->valueStackSize > 0x10000)){continue;}
			if((c->valueStack == NULL) || (c->closureStack == NULL)){continue;}
			lThreadGCMark(p);
		}
		if(p < lStartOfHeap){continue;}
		if(p > lEndOfHeap){continue;}
		if(couldBeArray(p)){

		}else if(couldBeBCA(p)){
			lBytecodeArrayMark(p);
		}else if(couldBeClosure(p)){
			lClosureGCMark(p);
		}else if(couldBeNFunc(p)){
			lNFuncGCMark(p);
		}else if(couldBeString(p)){
			lStringGCMark(p);
		}else if(couldBeTree(p)){
			lTreeGCMark(p);
		}else if(couldBeVal(p)){
			lValGCMark(p);
		}
	}
}
