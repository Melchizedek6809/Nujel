/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../lib/api.h"
#include "misc.h"
#include "operation/environment.h"
#include "operation/io.h"
#include "operation/readline.h"

#include <stdlib.h>

#ifndef __WATCOMC__
  #include <signal.h>
#endif
#include "../vendor/getline/getline.h"

extern u8 binlib_no_data[];
lClosure *mainClosure;

/* Evaluate an expression without expanding/compiling it, probably
 * not what you want, run does expansion/compilation before invoking
 * the VM. */
static void *evalRaw(void *cl, void *body){
	return lEval((lClosure *)cl,(lVal *)body);
}

#ifdef __EMSCRIPTEN__

/* To be used for the WASM REPL, since we don't run continuously there */
const char *run(const char *line){
	const int SP = lRootsGet();
	lVal *exp = RVP(lList(2,RVP(lValSym("repl/wasm")),RVP(lValString(line))));
	lVal *v = lExceptionTryExit(evalRaw,mainClosure,exp);
	const char *ret = v ? lReturnDisplayVal(v) : "";
	lRootsRet(SP);
	return ret;
}
#endif

/* Return a new root Closure, with all native functions in place */
static lClosure *createRootClosure(){
	lClosure *c;
	c = lNewRoot();
	lOperationsIO(c);
	lOperationsReadline(c);
	lnfDo(c,lRead((const char *)binlib_no_data));
	lGarbageCollect();
	return c;
}

/* Parse options that might radically alter runtime behaviour, like running
 * without the stdlib (probably for using an alternative build of the stdlib ) */
static lClosure *parsePreOptions(int argc, char *argv[]){
	lClosure *c     = NULL;
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
	if(lVerbose){
		pf("sizeof(vec): %u\n",     (i64)sizeof(vec));
		pf("sizeof(lClosure): %u\n",(i64)sizeof(lClosure));
		pf("sizeof(lVal): %u\n",    (i64)sizeof(lVal));
		pf("sizeof(lArray): %u\n",  (i64)sizeof(lArray));
		pf("sizeof(lString): %u\n", (i64)sizeof(lString));
		pf("sizeof(lTree): %u\n",   (i64)sizeof(lTree));
		pf("sizeof(jmp_buf): %u\n", (i64)sizeof(jmp_buf));
	}

	if(c == NULL){c = createRootClosure();}

	if(lVerbose){
		pf("\n\nRoot Closure Data Size: %u\n", (i64)lTreeSize(c->data));
	}
	return c;
}

/* Initialize the Nujel context with an stdlib as well
 * as parsing arguments passed to the runtime */
void initNujel(int argc, char *argv[], lClosure *c){
	lVal *ret = NULL;
	const int SP = lRootsGet();
	initEnvironmentMap(c);
	for(int i = argc-1; i >= 0; i--){
		ret = lCons(lValString(argv[i]), ret);
	}
	lRootsRet(SP);
	RVP(ret);
	ret = lCons(lValSym("repl/init"), ret);
	lRootsRet(SP);
	RVP(ret);
	lExceptionTryExit(evalRaw,c,ret);
	mainClosure = c;
}


#ifndef __WATCOMC__
/* Signal handler that enabled using C-c to break out of
 * an infinite loop */
static void breakSignalHandler(int sig){
	(void)sig;
	breakQueued = true;
}

/* Set up a bunch of signal handlers */
static void initSignalHandlers(){
	signal(SIGINT, breakSignalHandler);
}
#endif


int main(int argc, char *argv[]){
	(void)argc; (void)argv;
	#ifndef __WATCOMC__
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	initSignalHandlers();
	#endif
	lInit();
	setIOSymbols();

	initNujel(argc,argv,parsePreOptions(argc,argv));

	return 0;
}
