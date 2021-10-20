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

extern char binlib_nuj_data[];

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

static void *loadFile(const char *filename,size_t *len){
	FILE *fp;
	size_t filelen,readlen,read;
	u8 *buf = NULL;

	fp = fopen(filename,"rb");
	if(fp == NULL){return NULL;}

	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	fseek(fp,0,SEEK_SET);

	buf = malloc(filelen+1);
	if(buf == NULL){return NULL;}

	readlen = 0;
	while(readlen < filelen){
		read = fread(buf+readlen,1,filelen-readlen,fp);
		if(read == 0){
			free(buf);
			return NULL;
		}
		readlen += read;
	}
	fclose(fp);
	buf[filelen] = 0;

	*len = filelen;
	return buf;
}

static void saveFile(const char *filename,const void *buf, size_t len){
	FILE *fp;
	size_t written,wlen = 0;
	#if defined (__EMSCRIPTEN__)
	(void)filename;
	(void)buf;
	(void)len;
	return;
	#endif

	fp = fopen(filename,"wb");
	if(fp == NULL){return;}

	while(wlen < len){
		written = fwrite(buf+wlen,1,len-wlen,fp);
		if(written == 0){return;}
		wlen += written;
	}
	fclose(fp);
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

static lVal *lnfQuit(lClosure *c, lVal *v){
	(void)c;
	exit(castToInt(lCar(v),0));
	return NULL;
}

static lVal *lnfInput(lClosure *c, lVal *v){
	(void)c;
	const char *prompt = castToString(lCar(v),NULL);
	if(prompt != NULL){
		printf("%s",prompt);
	}
	char buf[4096];
	if(fgets(buf,sizeof(buf),stdin) == NULL){
		return NULL;
	}
	return lValString(buf);
}

static lVal *lnfPrint(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return v;}
	lDisplayVal(lCar(v));
	return NULL;
}

static lVal *lnfError(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return v;}
	lDisplayErrorVal(lCar(v));
	return NULL;
}

static lVal *lnfReadFile(lClosure *c, lVal *v){
	(void)c;
	const char *filename = castToString(lCar(v),NULL);
	if(filename == NULL){return NULL;}
	size_t len = 0;
	const char *data = loadFile(filename,&len);
	lVal *ret = lValString(data);
	free((void *)data);
	return ret;

}

static lVal *lnfWriteFile(lClosure *c, lVal *v){
	(void)c;
	const char *filename = castToString( lCar(v),NULL);
	const char *content  = castToString(lCadr(v),NULL);
	if(filename == NULL){return NULL;}
	if(content  == NULL){return NULL;}
	size_t len = strnlen(content,1<<20);
	saveFile(filename,content,len);
	return NULL;

}

void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}

static void addNativeFuncs(lClosure *c){
	lAddNativeFunc(c,"error",     "[...args]",         "Prints ...args to stderr",                           lnfError);
	lAddNativeFunc(c,"print",     "[...args]",         "Displays ...args",                                   lnfPrint);
	lAddNativeFunc(c,"input",     "[]",                "Reads in a line of user input and returns it",       lnfInput);
	lAddNativeFunc(c,"quit",      "[a]",               "Exits with code a",                                  lnfQuit);
	lAddNativeFunc(c,"exit",      "[a]",               "Quits with code a",                                  lnfQuit);
	lAddNativeFunc(c,"file/read", "[filename]",        "Load FILENAME and return the contents as a string",  lnfReadFile);
	lAddNativeFunc(c,"file/write","[filename content]","Writes CONTENT into FILENAME",                       lnfWriteFile);
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
					lGCVerbose = true;
					break;
				}
			}
		}
	}
	lClosure *c;
	if(loadStdLib){
		c = lClosureNewRoot();
		addNativeFuncs(c);
		lEval(c,lWrap(lRead((const char *)binlib_nuj_data)));
		lGarbageCollect();
	}else{
		c = lClosureNewRootNoStdLib();
		addNativeFuncs(c);
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
