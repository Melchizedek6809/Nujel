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
	spf(buf, &buf[sizeof(buf)], "%v", v);
	return buf;
}
#endif

/* Return a new root Closure, with all native functions in place */
static lClosure *createRootClosure(){
	lClosure *c = lNewRoot();
	lOperationsIO(c);
	lOperationsReadline(c);
	mainClosure = lLoad(c, (const char *)binlib_no_data);
	return c;
}

/* Parse options that might radically alter runtime behaviour, like running
 * without the stdlib (probably for using an alternative build of the stdlib ) */
static lClosure *parsePreOptions(int argc, char *argv[]){
	lClosure *c = NULL;
	for(int i=1;i<argc;i++){
		if(argv[i][0] == '-'){
			for(const char *opts = &argv[i][1];*opts && (*opts != '-');opts++){
				switch(*opts){
				case 'v':
					lVerbose = true;
					break;
				}
			}
		}
	}
	if(c == NULL){c = createRootClosure();}
	return c;
}

lVal *replInitSym;

/* Initialize the Nujel context with an stdlib as well
 * as parsing arguments passed to the runtime */
int initNujel(int argc, char *argv[], lClosure *c){
	lVal * volatile ret = NULL;
	initEnvironmentMap(c);
	for(int i = argc-1; i >= 0; i--){
		ret = lCons(lValString(argv[i]), ret);
	}
	replInitSym = lValSym("repl/init");
	lVal *fun = lGetClosureSym(c, replInitSym->vSymbol);
	lApply(c, ret, fun);
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

	return initNujel(argc,argv,parsePreOptions(argc,argv));
}
