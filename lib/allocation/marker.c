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
	if((u64)p & (sizeof(void *) - 1)){return false;}
	if(p < (void *)lArrayList){return false;}
	if(p > (void *)&lArrayList[ARR_MAX]){return false;}
	return true;
}

static bool couldBeClosure(void *p){
	if((u64)p & (sizeof(void *) - 1)){return false;}
	if(p < (void *)lClosureList){return false;}
	if(p > (void *)&lClosureList[CLO_MAX]){return false;}
	return true;
}

static bool couldBeNFunc(void *p){
	if((u64)p & (sizeof(void *) - 1)){return false;}
	if(p < (void *)lNFuncList){return false;}
	if(p > (void *)&lNFuncList[NFN_MAX]){return false;}
	return true;
}

static bool couldBeString(void *p){
	if((u64)p & (sizeof(void *) - 1)){return false;}
	if(p < (void *)lStringList){return false;}
	if(p > (void *)&lStringList[STR_MAX]){return false;}
	return true;
}

static bool couldBeTree(void *p){
	if((u64)p & (sizeof(void *) - 1)){return false;}
	if(p < (void *)lTreeList){return false;}
	if(p > (void *)&lTreeList[TRE_MAX]){return false;}
	return true;
}

static bool couldBeVal(void *p){
	if((u64)p & (sizeof(void *) - 1)){return false;}
	if(p < (void *)lValList){return false;}
	if(p > (void *)&lValList[VAL_MAX]){return false;}
	return true;
}

static bool couldBeBCA(void *p){
	if((u64)p & (sizeof(void *) - 1)){return false;}
	if(p < (void *)lBytecodeArrayList){return false;}
	if(p > (void *)&lBytecodeArrayList[BCA_MAX]){return false;}
	return true;
}

extern bool verboseMarking;

void lGCMarkStack(void *endOfStack){
	const void *start = lStartOfStack;
	//verboseMarking = true;
	//pf("Marking the stack:\n%p\n%p\n%p\n\n", start, endOfStack, checkForVal);
	for(void *p = endOfStack; p < start; p+= sizeof(void *)){
		const u64 vv = *((u64 *)p);
		if(vv == 0xbaddbeefcafebabe){
			lThread *c = (lThread *)p;
			if((c->callStackSize & 0xF) || (c->valueStackSize & 0xF)){continue;}
			if((c->callStackSize > 0x10000) || (c->callStackSize < 16)){continue;}
			if((c->valueStackSize > 0x10000) || (c->valueStackSize < 16)){continue;}
			if((c->valueStack == NULL) || (c->callStack == NULL)){continue;}
			lThreadGCMark(p);
		}
		void *v = *((void **)p);
		if(v < lStartOfHeap){continue;}
		if(v > lEndOfHeap){continue;}
		if(couldBeArray(v)){
			lArrayGCMark(v);
		}else if(couldBeBCA(v)){
			lBytecodeArrayMark(v);
		}else if(couldBeClosure(v)){
			lClosureGCMark(v);
		}else if(couldBeNFunc(v)){
			lNFuncGCMark(v);
		}else if(couldBeString(v)){
			lStringGCMark(v);
		}else if(couldBeTree(v)){
			lTreeGCMark(v);
		}else if(couldBeVal(v)){
			lValGCMark(v);
		}
	}
	verboseMarking = false;
}
