/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#ifndef COSMOPOLITAN_H_
	#include <stdarg.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>

	#ifdef __MINGW32__
		#include <windows.h>
		#include <shlobj.h>
	#else
		#include "../vendor/bestline/bestline.h"
	#endif
#else
	#include "../vendor/bestline/bestline.h"
#endif

#include "../lib/api.h"
#include "misc.h"
#include "operator/io.h"

extern char binlib_no_data[];

#ifdef __MINGW32__
	static void bestlineHistoryLoad(const char *path){(void)path;}
	static void bestlineHistorySave(const char *path){(void)path;}
	static void bestlineHistoryAdd (const char *line){(void)line;}

	static char *bestline(const char *prompt){
		static char buf[4096];
		printf("%s",prompt);
		fflush(stdout);
		if(fgets(buf,sizeof(buf),stdin) == NULL){
			return NULL;
		}
		return buf;
	}
#endif

void lGUIWidgetFree(lVal *v){
	(void)v;
}

void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}

const char *getHistoryPath(){
	static char buf[512];

	#ifdef __MINGW32__
	char home[512];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, home);
	if(result != S_OK){
		return NULL;
	}
	#else
	const char* home = getenv("HOME");
	if(!home){
		return NULL;
	}
	#endif

	if(snprintf(buf,sizeof(buf),"%s/.nujel_history",home) <= 0){ // snprintf instead of strcpy/strcat
		fprintf(stderr,"Can't create historyPath, maybe your $HOME is too big?\n");
		return NULL;
	}
	return buf;
}

void doRepl(lClosure *c){
	const char *historyPath = getHistoryPath();
	if(historyPath){
		bestlineHistoryLoad(historyPath);
	}
	lVal *lastlsym = lValSym("lastl");
	lVal *lastl    = lDefineClosureSym(c, lastlsym->vSymbol);
	while(1){
		char *str = bestline("> ");
		if(str == NULL){
			printf("\nBye!\n");
			return;
		}
		bestlineHistoryAdd(str);
		if(historyPath){
			bestlineHistorySave(historyPath);
		}
		lVal *v = lEval(c,lWrap(lRead(str)));
		if(v != NULL){
			lWriteVal(v);
		}else{
			printf("\n");
		}
		lGarbageCollect();
		lVal *tmp = lValString(str);
		if((tmp != NULL) && (lastl != NULL)){lastl->vList.car = tmp;}
	}
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
		addNativeFuncs(c);
		lEval(c,lWrap(lRead((const char *)binlib_no_data)));
		lGarbageCollect();
	}else{
		c = lClosureNewRootNoStdLib();
		addNativeFuncs(c);
	}

	if(lVerbose){
		printf("sizeof(lClosure): %u\n",(uint)sizeof(lClosure));
		printf("sizeof(lVal): %u\n",    (uint)sizeof(lVal));
		printf("sizeof(lArray): %u\n",  (uint)sizeof(lArray));
		printf("sizeof(lString): %u\n", (uint)sizeof(lString));
		printf("sizeof(lTree): %u\n", (uint)sizeof(lTree));
	}
	return c;
}

int main(int argc, char *argv[]){
	int eval = 0;
	int repl = 1;
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	lInit();

	lClosure *c = parsePreOptions(argc,argv);
	for(int i=1;i<argc;i++){
		size_t len;
		char *str = argv[i];
		if(argv[i][0] == '-'){
			if(argv[i][1] == 'e'){
				eval = 1;
				continue;
			}else if(argv[i][1] == 'x'){
				eval = 2;
				continue;
			}else if(argv[i][1] == '-'){
				repl = 1;
				continue;
			}else if(argv[i][1] == 'n'){
				continue;
			}else{
				break;
			}
		}
		if(!eval){
			str = loadFile(argv[i],&len);
		}
		lVal *v = lEval(c,lWrap(lRead(str)));
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
