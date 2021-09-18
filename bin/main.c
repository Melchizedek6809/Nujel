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

#include "../lib/common.h"
#include "../lib/nujel.h"
#include "../lib/garbage-collection.h"
#include "../lib/casting.h"
#include "../lib/reader.h"
#include "../lib/datatypes/closure.h"
#include "../lib/datatypes/native-function.h"
#include "../lib/datatypes/string.h"
#include "../lib/datatypes/symbol.h"
#include "../lib/operations/string.h"

extern char binlib_nuj_data[];

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

void doRepl(lClosure *c){
	static char str[4096];
	lVal *lastlsym = lValSym("lastl");
	lVal *lastl    = lDefineClosureSym(c - lClosureList, lvSym(lastlsym->vCdr));
	lVal *repl     = lCons(lValSym("repl-prompt"),NULL);
	repl->flags |= lfNoGC;
	while(1){
		lEval(c,repl);
		fflush(stdout);
		if(fgets(str,sizeof(str),stdin) == NULL){
			printf("\nBye!\n");
			return;
		}
		lVal *v = lEval(c,lWrap(lRead(str)));
		lWriteVal(v);
		lGarbageCollect();
		lVal *tmp = lValString(str);
		if((tmp != NULL) && (lastl != NULL)){lastl->vList.car = tmp;}
	}
}

static lVal *lnfQuit(lClosure *c, lVal *v){
	int ecode = 0;
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v->vList.car));
		if((t != NULL) && (t->type == ltInt)){
			ecode = t->vInt;
		}
	}
	exit(ecode);
	return NULL;
}

static lVal *lnfInput(lClosure *c, lVal *v){
	static char buf[512];
	if(v != NULL){
		lVal *t = lnfCat(c,v);
		if((t != NULL) && (t->type == ltString)){
			printf("%s",lStrData(t));
		}
	}
	if(fgets(buf,sizeof(buf),stdin) == NULL){
		return NULL;
	}
	return lValString(buf);
}

static lVal *lnfPrint(lClosure *c, lVal *v){
	if(v == NULL){return v;}
	lVal *t = NULL;
	if(v->type == ltPair){
		t = lEval(c,v->vList.car);
	}else{
		t = lEval(c,v);
	}
	lDisplayVal(t);
	return NULL;
}

static lVal *lnfError(lClosure *c, lVal *v){
	if(v == NULL){return v;}
	lVal *t = NULL;
	if(v->type == ltPair){
		t = lEval(c,v->vList.car);
	}else{
		t = lEval(c,v);
	}
	lDisplayErrorVal(t);
	return NULL;
}

static lVal *lnfReadFile(lClosure *c, lVal *v){
	const char *filename = NULL;
	size_t len = 0;

	v = getLArgS(c,v,&filename);
	if(filename == NULL){return NULL;}
	const char *data = loadFile(filename,&len);
	lVal *ret = lValString(data);
	free((void *)data);

	return ret;

}

static lVal *lnfWriteFile(lClosure *c, lVal *v){
	const char *filename = NULL;
	const char *content = NULL;

	v = getLArgS(c,v,&filename);
	v = getLArgS(c,v,&content);
	if(filename == NULL){return NULL;}
	if(content == NULL) {return NULL;}
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
	lAddNativeFunc(c,"display",   "[...args]",         "Prints ...args",                                     lnfPrint);
	lAddNativeFunc(c,"input",     "[]",                "Reads in a line of user input and returns it",       lnfInput);
	lAddNativeFunc(c,"quit",      "[a]",               "Exits with code a",                                  lnfQuit);
	lAddNativeFunc(c,"exit",      "[a]",               "Quits with code a",                                  lnfQuit);
	lAddNativeFunc(c,"file/load file/read",  "[filename]",        "Load FILENAME and return the contents as a string",  lnfReadFile);
	lAddNativeFunc(c,"file/save file/write", "[filename content]","Writes CONTENT into FILENAME",                       lnfWriteFile);
}

int main(int argc, char *argv[]){
	int eval = 0;
	int repl = 1;
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	lInit();
	lClosure *c = lClosureNewRoot();
	addNativeFuncs(c);
	lEval(c,lWrap(lRead((const char *)binlib_nuj_data)));
	lGarbageCollect();

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
			}else{
				break;
			}
		}
		if(!eval){str = loadFile(argv[i],&len);}
		lVal *v = lEval(c,lWrap(lRead(str)));
		if((i == argc-1) && !repl && (eval != 2)){lWriteVal(v);}
		lGarbageCollect();

		if(!eval){
			free(str);
			eval = 0;
		}
		repl = 0;
	}
	if(repl){
		doRepl(c);
	}
	lClosureFree(c - lClosureList);
	return 0;
}
