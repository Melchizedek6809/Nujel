/*
 * Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "readline.h"
#include "../misc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__MINGW32__) || defined(__MSYS__)
	#include <windows.h>
	#include <shlobj.h>
#endif
#include "../../vendor/getline/getline.h"
#define BUF_SIZE (1 << 14)
static void bestlineHistoryLoad(const char *path){(void)path;}
static void bestlineHistorySave(const char *path){(void)path;}
static void bestlineHistoryAdd (const char *line){(void)line;}
static char *bestline(const char *prompt){
	char *buf = NULL;
	size_t bufsize = 0;

	fputs(prompt, stdout);
	const ssize_t ret = getline(&buf,&bufsize,stdin);

	if(ret >= 0){
		buf[MIN(bufsize-1,(size_t)ret)] = 0;
		return buf;
	}else{
		return NULL;
	}
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

	spf(buf,&buf[sizeof(buf)],"%s/.nujel_history",home);
	return buf;
}

static lVal *lnfReadline(lClosure *c, lVal *v){
	(void) c;
	const char *prompt = castToString(lCar(v),"> ");
	char *line = bestline(prompt);
	lVal *ret = lValString(line);
	free(line);
	return ret;
}

static lVal *lnfReadlineHistoryPath(lClosure *c, lVal *v){
	(void) c; (void) v;

	return lValString(getHistoryPath());
}

static lVal *lnfReadlineHistoryAdd(lClosure *c, lVal *v){
	(void) c;
	const char *line = castToString(lCar(v),NULL);
	if(line == NULL){return NULL;}
	bestlineHistoryAdd(line);
	return lCar(v);
}

static lVal *lnfReadlineHistoryLoad(lClosure *c, lVal *v){
	(void) c;
	const char *path = castToString(lCar(v),NULL);
	if(path == NULL){return NULL;}
	bestlineHistoryLoad(path);
	return lValBool(true);
}

static lVal *lnfReadlineHistorySave(lClosure *c, lVal *v){
	(void) c;
	const char *path = castToString(lCar(v),NULL);
	if(path == NULL){return NULL;}
	bestlineHistorySave(path);
	return lValBool(true);
}

void lOperationsReadline(lClosure *c){
	lAddNativeFunc(c,"readline",              "[prompt]", "Read a line of input in a user friendly way after writing PROMPT", lnfReadline);
	lAddNativeFunc(c,"readline/history/path", "[]",       "Return the default path to the command history",                   lnfReadlineHistoryPath);
	lAddNativeFunc(c,"readline/history/load", "[path]",   "Load command history from PATH",                                   lnfReadlineHistoryLoad);
	lAddNativeFunc(c,"readline/history/save", "[path]",   "Save command history into PATH",                                   lnfReadlineHistorySave);
	lAddNativeFunc(c,"readline/history/add",  "[line]",   "Add an entry into the command history without saving",             lnfReadlineHistoryAdd);
}
