/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "io.h"
#include "../misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void addNativeFuncs(lClosure *c){
	lAddNativeFunc(c,"error",     "[...args]",         "Prints ...args to stderr",                           lnfError);
	lAddNativeFunc(c,"print",     "[...args]",         "Displays ...args",                                   lnfPrint);
	lAddNativeFunc(c,"input",     "[]",                "Reads in a line of user input and returns it",       lnfInput);
	lAddNativeFunc(c,"quit",      "[a]",               "Exits with code a",                                  lnfQuit);
	lAddNativeFunc(c,"exit",      "[a]",               "Quits with code a",                                  lnfQuit);
	lAddNativeFunc(c,"file/read", "[filename]",        "Load FILENAME and return the contents as a string",  lnfReadFile);
	lAddNativeFunc(c,"file/write","[filename content]","Writes CONTENT into FILENAME",                       lnfWriteFile);
}
