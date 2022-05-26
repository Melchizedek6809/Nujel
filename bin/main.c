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
	mainClosure = lNewRoot();
	lOperationsIO(mainClosure);
	lOperationsReadline(mainClosure);
	mainClosure = lLoad(mainClosure, (const char *)binlib_no_data);
	lGarbageCollect();
	return mainClosure;
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
	if(lVerbose){
		pf("sizeof(vec): %u\n",     (i64)sizeof(vec));
		pf("sizeof(lClosure): %u\n",(i64)sizeof(lClosure));
		pf("sizeof(lVal): %u\n",    (i64)sizeof(lVal));
		pf("sizeof(lArray): %u\n",  (i64)sizeof(lArray));
		pf("sizeof(lString): %u\n", (i64)sizeof(lString));
		pf("sizeof(lTree): %u\n",   (i64)sizeof(lTree));
		pf("sizeof(jmp_buf): %u\n", (i64)sizeof(jmp_buf));
		pf("Root Size: %u\n",       (i64)lTreeSize(c->data));
	}
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

void (*mainRootsMarkerChain)() = NULL;
void mainMarker(){
	lClosureGCMark(mainClosure);
	lValGCMark(replInitSym);
	if(mainRootsMarkerChain){mainRootsMarkerChain();}
}

int main(int argc, char *argv[]){
	(void)argc; (void)argv;
	volatile void *lStartOfStackTestVal = NULL;
	lSetStartOfStack(&lStartOfStackTestVal);
	mainRootsMarkerChain = rootsMarkerChain;
	rootsMarkerChain = mainMarker;

	#ifndef __WATCOMC__
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	#endif
	lInit();
	setIOSymbols();

	return initNujel(argc,argv,parsePreOptions(argc,argv));
}
