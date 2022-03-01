/*
 * Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "../lib/api.h"
#include "misc.h"
#include "operation/environment.h"
#include "operation/io.h"
#include "operation/readline.h"
#include "../vendor/getline/getline.h"

extern u8 binlib_no_data[];
lClosure *mainClosure;
bool loadStdLib = true;

#ifdef __EMSCRIPTEN__
static void *runRaw(void *cl, void *body){
	return lEval((lClosure *)cl,(lVal *) body);
}

/* To be used for the WASM REPL, since we don't run continuously there */
const char *run(const char *line){
	const int SP = lRootsGet();
	lVal *exp = RVP(lList(2,RVP(lValSym("repl/wasm")),RVP(lValString(line))));
	lVal *v = lExceptionTryExit(runRaw,mainClosure,exp);
	const char *ret = v ? lReturnDisplayVal(v) : "";
	lRootsRet(SP);
	return ret;
}
#endif

/* Read and Eval str directly */
/* DOES NOT EXPAND MACROS */
static void *readEvalStringRaw(void *cl, void *str){
	lClosure *c = (lClosure *)cl;
	const char *expr = str;
	lVal *v = lnfDo(c,lRead(expr));
	return v;
}

static char *getLine(){
	char *buf = NULL;
	size_t bufsize = 0;

	fputs("RAW> ", stdout);
	const ssize_t ret = getline(&buf,&bufsize,stdin);

	if(ret >= 0){
		buf[MIN(bufsize-1,(size_t)ret)] = 0;
		return buf;
	}else{
		return NULL;
	}
}

static void replRaw(lClosure *c){
	char *line = NULL;
	while(true){
		line = getLine();
		if(line == NULL){break;}
		lVal *v = readEvalStringRaw(c, line);
		pf("  %V\n", v);
	}
	pf("\n");
	exit(0);
}

/* Return a new root Closure, with all native functions in place */
static lClosure *createRootClosure(){
	lClosure *c;
	if(loadStdLib){
		c = lNewRoot();
		lOperationsIO(c);
		lOperationsReadline(c);
		lnfDo(c,lRead((const char *)binlib_no_data));
		lGarbageCollect();
	}else{
		c = lNewRootNoStdLib();
		lOperationsIO(c);
		lOperationsReadline(c);
	}
	return c;
}

/* Parse options that might radically alter runtime behaviour, like running
 * without the stdlib (probably for using an alternative build of the stdlib ) */
static lClosure *parsePreOptions(int argc, char *argv[]){
	bool readNext   = false;
	lClosure *c     = NULL;
	for(int i=1;i<argc;i++){
		if(readNext){
			if(c == NULL){c = createRootClosure();}
			size_t len = 0;
			char *str = loadFile(argv[i],&len);
			lExceptionTryExit(readEvalStringRaw,c,str);
			free(str);
			readNext = false;
		}else if(argv[i][0] == '-'){
			for(const char *opts = &argv[i][1];*opts;opts++){
				switch(*opts){
				case 'r':
					readNext = true;
					break;
				case 'n':
					loadStdLib = false;
					break;
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
		pf("\n\nRoot Closure Data Size: %u\n", (i64)lTreeSize(c->data));
	}
	return c;
}

static void *evalRaw(void *cl, void *body){
	return lEval((lClosure *)cl,(lVal *)body);
}

void initNujel(int argc, char *argv[], lClosure *c){
	lVal *ret = NULL;
	const int SP = lRootsGet();
	initEnvironmentMap(c);
	for(int i = argc-1; i >= 0; i--){
		ret = lCons(lValString(argv[i]), ret);
	}
	lRootsRet(SP);
	RVP(ret);
	if(loadStdLib){
		ret = lCons(lValSym("repl/init"), ret);
	}else{
		replRaw(c);
	}
	lRootsRet(SP);
	RVP(ret);
	lExceptionTryExit(evalRaw,c,ret);
	mainClosure = c;
}

static void breakSignalHandler(int sig){
	(void)sig;
	breakQueued = true;
}

static void initSignalHandlers(){
	signal(SIGINT, breakSignalHandler);
}

int main(int argc, char *argv[]){
	(void)argc; (void)argv;
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	initSignalHandlers();
	lInit();
	setIOSymbols();

	initNujel(argc,argv,parsePreOptions(argc,argv));

	return 0;
}
