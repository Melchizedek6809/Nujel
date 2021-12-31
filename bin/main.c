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
#include "operation/readline.h"
#include "operation/io.h"

extern unsigned char binlib_no_data[];
lClosure *mainClosure;

#ifdef __EMSCRIPTEN__
void *runRaw(void *cl, void *body){
	return lEval((lClosure *)cl,(lVal *) body);
}

/* To be used for the WASM REPL, since we don't run continuously there */
const char *run(const char *line){
	const int SP = lRootsGet();
	lVal *exp = RVP(lList(2,RVP(lValSym("repl/wasm")),RVP(lValString(line))));
	lVal *v = lExceptionTry(runRaw,mainClosure,exp);
	const char *ret = v ? lReturnDisplayVal(v) : "";
	lRootsRet(SP);
	return ret;
}
#endif

static void *readEvalStringRaw(void *cl, void *str){
	lClosure *c = (lClosure *)cl;
	const char *expr = str;
	lVal *v = lnfDo(c,lRead(expr));
	return v;
}

static lClosure *createRoolClosure(bool loadStdLib){
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
 * without the stdlib (probably for using an alternative build of the stdlib )
 */
static lClosure *parsePreOptions(int argc, char *argv[]){
	bool loadStdLib = true;
	bool readNext   = false;
	lClosure *c     = NULL;
	for(int i=1;i<argc;i++){
		if(readNext){
			if(c == NULL){c = createRoolClosure(loadStdLib);}
			size_t len = 0;
			char *str = loadFile(argv[i],&len);
			lExceptionTry(readEvalStringRaw,c,str);
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
	if(c == NULL){c = createRoolClosure(loadStdLib);}

	if(lVerbose){
		pf("sizeof(lClosure): %u\n",sizeof(lClosure));
		pf("sizeof(lVal): %u\n",    sizeof(lVal));
		pf("sizeof(lArray): %u\n",  sizeof(lArray));
		pf("sizeof(lString): %u\n", sizeof(lString));
		pf("sizeof(lTree): %u\n",   sizeof(lTree));
		pf("sizeof(jmp_buf): %u\n", sizeof(jmp_buf));
		pf("\n\nRoot Closure Data Size: %u\n", lTreeSize(c->data));
	}
	return c;
}

static void *evalRaw(void *cl, void *body){
	return lEval((lClosure *)cl,(lVal *)body);
}


void initNujel(int argc, char *argv[], lClosure *c){
	lVal *ret = NULL;
	const int SP = lRootsGet();
	for(int i = argc-1; i >= 0; i--){
		ret = lCons(lValString(argv[i]), ret);
	}
	lRootsRet(SP);
	RVP(ret);
	ret = lCons(lValSym("repl/init"), ret);
	lRootsRet(SP);
	RVP(ret);
	lExceptionTry(evalRaw,c,ret);
	mainClosure = c;
}

void breakSignalHandler(int sig){
	(void)sig;
	breakQueued = true;
}

void initSignalHandlers(){
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
