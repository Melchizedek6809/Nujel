/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

#ifndef _MSC_VER
#include <unistd.h>
#endif

/* Add environment key/value pair to tree T */
static void addVar(const char *e, lMap *t){
	int endOfKey, endOfString;
	for(endOfKey=0;e[endOfKey] != '=';endOfKey++){}
	for(endOfString=endOfKey+1;e[endOfString];endOfString++){}
	lSymbol *sym = lSymSL(e,endOfKey);
	lVal v = lValString(&e[endOfKey+1]);
	lMapSet(t, lValKeywordS(sym), v);
}

#if (defined(__MSYS__)) || (defined(__MINGW32__)) || (defined(_WIN32))
#include <windows.h>

/* Windows specific - add Environment args to `environment/variables` */
void lRedefineEnvironment(lClosure *c){
	lTree *t = NULL;
	LPCH env = GetEnvironmentStrings();
	while(*env){
		addVar(env,t);
		while(*env++){}
	}
	lDefineClosureSym(c,lSymS("System/Environment"), lValMap(t));
}

#else
extern char **environ;
/* Add Environment args to `environment/variables` */
void lRedefineEnvironment(lClosure *c){
	lMap *t = lMapAllocRaw();
	#ifdef __wasi__
	t = addVar("PATH=",t); // Necessary so that tests don't fail
	#endif
	for(int i=0;environ[i];i++){
		addVar(environ[i], t);
	}
	lDefineClosureSym(c,lSymS("System/Environment"), lValMap(t));
}
#endif
