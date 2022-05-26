/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "marker.h"
#include "garbage-collection.h"
#include "symbol.h"
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
	lStartOfHeap = MIN(lStartOfHeap, (void *)lSymbolList);

	lEndOfHeap = &lArrayList[ARR_MAX];
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lClosureList[CLO_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lNFuncList[NFN_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lStringList[STR_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lTreeList[TRE_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lValList[VAL_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lBytecodeArrayList[BCA_MAX]);
	lEndOfHeap = MAX(lEndOfHeap, (void *)&lSymbolList[SYM_MAX]);
}

static bool couldBeArray(void *p){
	const u64 d = (lArray *)p - lArrayList;
	return d < lArrayMax;
}

static bool couldBeClosure(void *p){
	const u64 d = (lClosure *)p - lClosureList;
	return d < lClosureMax;
}

static bool couldBeNFunc(void *p){
	const u64 d = (lNFunc *)p - lNFuncList;
	return d < lNFuncMax;
}

static bool couldBeString(void *p){
	const u64 d = (lString *)p - lStringList;
	return d < lStringMax;
}

static bool couldBeTree(void *p){
	const u64 d = (lTree *)p - lTreeList;
	return d < lTreeMax;
}

static bool couldBeVal(void *p){
	const u64 d = (lVal *)p - lValList;
	return d < lValMax;
}

static bool couldBeBCA(void *p){
	const u64 d = (lBytecodeArray *)p - lBytecodeArrayList;
	return d < lBytecodeArrayMax;
}

static bool couldBeSymbol(void *p){
	const u64 d = (lSymbol *)p - lSymbolList;
	return d < lSymbolMax;
}

bool shouldBeMarked(void *v){
	if(v < lStartOfHeap){return false;}
	if(v > lEndOfHeap){return false;}
	if(couldBeArray(v)){
		return true;
	}else if(couldBeBCA(v)){
		return true;
	}else if(couldBeClosure(v)){
		return true;
	}else if(couldBeNFunc(v)){
		return true;
	}else if(couldBeString(v)){
		return true;
	}else if(couldBeTree(v)){
		return true;
	}else if(couldBeVal(v)){
		return true;
	}else if(couldBeSymbol(v)){
		return true;
	}
	return false;
}

bool isReferencedInStack(void *needle, void *endOfStack){
	const void *start = lStartOfStack;
	for(void *p = endOfStack; p < start; p+= sizeof(void *)){
		void *v = *((void **)p);
		if(v == needle){return true;}
	}
	return false;
}

void lGCMarkStack(void *endOfStack){
	const void *start = lStartOfStack;
	for(void *p = endOfStack; p < start; p+= sizeof(void *)){
		const u64 vv = *((u64 *)p);
		if(vv == 0xbaddbeefcafebabe){
			lThread *c = (lThread *)p;
			if((c->closureStackSize & 3) || (c->valueStackSize & 7)){continue;}
			if((c->closureStackSize > 0x10000) || (c->valueStackSize > 0x10000)){continue;}
			if((c->valueStack == NULL) || (c->closureStack == NULL)){continue;}
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
		}else if(couldBeSymbol(v)){
			lSymbolGCMark(v);
		}
	}
}
