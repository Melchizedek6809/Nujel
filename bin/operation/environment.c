/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "io.h"
#include "../misc.h"

#include <unistd.h>

/* Add environment key/value pair to tree T */
static lTree *addVar(const char *e, lTree *t){
	int endOfKey, endOfString;
	for(endOfKey=0;e[endOfKey] != '=';endOfKey++){}
	for(endOfString=endOfKey+1;e[endOfString];endOfString++){}
	lSymbol *sym = RSYMP(lSymSL(e,endOfKey));
	lVal *v = RVP(lValStringNoCopy(&e[endOfKey+1], endOfString));
	return lTreeInsert(t, sym, v);
}

#if (defined(__MSYS__)) || (defined(__MINGW32__))

#include <windows.h>

/* Windows specific - add Environment args to `environment/variables` */
void initEnvironmentMap(lClosure *c){
	const int SP = lRootsGet();
	lTree *t = NULL;
	LPCH env = GetEnvironmentStrings();
	while(*env){
		t = RTP(addVar(env,t));
		while(*env++){}
	}
	lVal *et = RVP(lValTree(t));
	lDefineClosureSym(c,lSymS("environment/variables"),et);
	lRootsRet(SP);
}

#else
extern char **environ;

/* Add Environment args to `environment/variables` */
void initEnvironmentMap(lClosure *c){
	const int SP = lRootsGet();
	lTree *t = NULL;
	for(int i=0;environ[i];i++){
		t = RTP(addVar(environ[i],t));
	}
	lVal *env = RVP(lValTree(t));
	lDefineClosureSym(c,lSymS("environment/variables"),env);
	lRootsRet(SP);
}
#endif
