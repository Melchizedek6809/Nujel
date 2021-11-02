/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>

#include "../lib/api.h"
#include "misc.h"
#include "operation/readline.h"
#include "operation/io.h"

extern char binlib_no_data[];


void lGUIWidgetFree(lVal *v){
	(void)v;
}

void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}

void doRepl(lClosure *c){
	lVal *cmd = lRootsValPush(lCons(NULL,NULL));
	cmd->vList.car = lValSym("repl");
	lEval(c,cmd);
}

lClosure * parsePreOptions(int argc, char *argv[]){
	bool loadStdLib = true;
	for(int i=1;i<argc;i++){
		if(argv[i][0] == '-'){
			for(const char *opts = &argv[i][1];*opts;opts++){
				switch(*opts){
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
	lClosure *c;
	if(loadStdLib){
		c = lClosureNewRoot();
		lOperationsIO(c);
		lOperationsReadline(c);
		lnfDo(c,lRead((const char *)binlib_no_data));
		lGarbageCollect();
	}else{
		c = lClosureNewRootNoStdLib();
		lOperationsIO(c);
		lOperationsReadline(c);
	}

	if(lVerbose){
		printf("sizeof(lClosure): %u\n",(uint)sizeof(lClosure));
		printf("sizeof(lVal): %u\n",    (uint)sizeof(lVal));
		printf("sizeof(lArray): %u\n",  (uint)sizeof(lArray));
		printf("sizeof(lString): %u\n", (uint)sizeof(lString));
		printf("sizeof(lTree): %u\n", (uint)sizeof(lTree));
		printf("sizeof(jmp_buf): %u\n", (uint)sizeof(jmp_buf));
		printf("\n\nRoot Closure Data Size: %u\n",lTreeSize(c->data));
		lWriteVal(lValTree(c->data));
	}
	return c;
}

int main(int argc, char *argv[]){
	int eval = 0;
	int repl = 1;
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	lInit();
	setIOSymbols();

	lClosure *c = parsePreOptions(argc,argv);
	for(int i=1;i<argc;i++){
		size_t len;
		char *str = argv[i];
		if(argv[i][0] == '-'){
			for(int ii=1;argv[i][ii];ii++){
				switch(argv[i][ii]){
				case 'e':
					eval = 1;
					continue;
				case 'x':
					eval = 2;
					continue;
				case '-':
					repl = 1;
					continue;
				case 'n':
				case 'v':
					continue;
				default:
					fprintf(stderr,"Unknown option '%c', exiting.", argv[i][ii]);
					exit(1);
				}
			}
			continue;
		}
		if(!eval){
			str = loadFile(argv[i],&len);
		}
		lVal *v = lnfDo(c,lRead(str));
		if((i == argc-1) && !repl && (eval != 2)){lWriteVal(v);}

		if(!eval){
			free(str);
			eval = 0;
		}
		repl = 0;
	}

	if(repl){
		doRepl(c);
	}
	lClosureFree(c);
	return 0;
}
