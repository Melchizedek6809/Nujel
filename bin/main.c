/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#ifndef __WATCOMC__
  #include <signal.h>
#endif

extern u8 binlib_no_data[];
lClosure *mainClosure;


#ifdef __EMSCRIPTEN__

/* To be used for the WASM REPL, since we don't run continuously there */
const char *run(const char *line){
	static char buf[1<<16];
	lVal *funSym = lValSym("repl/wasm");
	lVal *fun = lGetClosureSym(mainClosure, funSym->vSymbol);
	lVal *v = lApply(mainClosure, lCons(lValString(line),NULL), fun);
	if(!v || v->type != ltString){return NULL;}
	return lStringData(v->vString);
}
#endif

/* Return a new root Closure, with all native functions in place */
static lClosure *createRootClosure(){
	lClosure *c = lNewRoot();
	lOperationsIO(c);
	lOperationsPort(c);
	lOperationsInit(c);
	mainClosure = lLoad(c, (const char *)binlib_no_data);
	return c;
}

/* Initialize the Nujel context with an stdlib as well
 * as parsing arguments passed to the runtime */
int initNujel(int argc, char *argv[], lClosure *c){
	lVal * volatile ret = NULL;
	initEnvironmentMap(c);
	for(int i = argc-1; i >= 0; i--){
		ret = lCons(lValString(argv[i]), ret);
	}
	lApply(c, ret, lResolveVal(c, "repl/init"));
	return 0;
}

int main(int argc, char *argv[]){
	(void)argc; (void)argv;
	#ifndef __WATCOMC__
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	#endif
	lInit();
	setIOSymbols();

	return initNujel(argc,argv,createRootClosure());
}
